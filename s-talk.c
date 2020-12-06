// TODO: both clients exit when one enters line "![ENTER]"
// TODO: test mem leak
// TODO: README

#include "list.h"
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>			// for close()
#include <stdbool.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>

#define MSG_MAX_LEN 1024

// shared lists for in/out messages
List *list_out;
List *list_in;

// mutexes to control list access (solving cs problem)
pthread_mutex_t lock_in;
pthread_mutex_t lock_out;

// condition variable to control UDP-send thread sending data and screen thread
pthread_cond_t cond_in;
pthread_cond_t cond_out;

// var for host name and port numbers
char *remote_host;
int my_port;
int remote_port;

// for testing "!<ENTER>"
bool over;

// thread
pthread_t tid[4];

// function for check
#define CHECKP(p) do {       \
    if (!(p)) {              \
        perror_and_exit(#p); \
    }                        \
} while (false)

// function for displaying error and exit
void perror_and_exit(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

// sender thread
void *sender(void *arg) {
    // set up socket
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(remote_port);

    // sock.sin_addr.s_addr = INADDR_ANY;
    struct hostent *hp = gethostbyname(remote_host);
    CHECKP(hp);
    // sock.sin_addr.s_addr = *(u_long *)hp->h_addr;
    sin.sin_addr.s_addr = *(long *) (hp->h_addr_list[0]);

    int socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    CHECKP(socketDescriptor >= 0);

    // while program still running, send msg to the other user
    while (true) {
        pthread_mutex_lock(&lock_out);
        while (0 == List_count(list_out)) {
            pthread_cond_wait(&cond_out, &lock_out);
        }
        char *message = (char *)List_remove(list_out);
        sendto(socketDescriptor, message, strlen(message), 0, (struct sockaddr *)&sin, sizeof(struct sockaddr_in));
        // printf("[sender] send %s\n", message);

        // free up memory
        free(message);
        pthread_mutex_unlock(&lock_out); 
    }

    // close socket when done
    close(socketDescriptor);
    return NULL;
}

// receiver thread
void *receiver(void *arg) {
    // set up socket
    int socketDescriptor = socket(AF_INET, SOCK_DGRAM, 0);
    CHECKP(socketDescriptor >= 0);

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(my_port);
    CHECKP(bind(socketDescriptor, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) >= 0);

    char line[MSG_MAX_LEN];
    socklen_t len;

    // while program still running, recv msg from the other user
    while (true) {
        int nb = recvfrom(socketDescriptor, line, MSG_MAX_LEN, 0, (struct sockaddr *)&sin, &len);
        line[nb] = '\0';
        if (!strcmp(line, "!")) {
            over = true;
            break;
        }
        // printf("[receiver] %s\n", line);

        pthread_mutex_lock(&lock_in);
        List_append(list_in, strdup(line));
        pthread_cond_signal(&cond_in);
        pthread_mutex_unlock(&lock_in);
    }

    // close socket when done
    close(socketDescriptor);
    return NULL;
}

// keyboard thread
void *keyboard(void *arg) {
    char line[MSG_MAX_LEN+1];
    while (true) {
        fgets(line, MSG_MAX_LEN, stdin);
        line[strlen(line)-1] = '\0';
        if (!strcmp(line, "!")) {
            over = true;
        }
        // printf("[keyboard] %s\n", line);
        
        pthread_mutex_lock(&lock_out);
        List_append(list_out, strdup(line));
        pthread_cond_signal(&cond_out);
        pthread_mutex_unlock(&lock_out); 
    }

    return NULL;
}

// printer thread
void *print(void *arg) {
    while (true) {
        pthread_mutex_lock(&lock_in);
        while (0 == List_count(list_in)) {
            pthread_cond_wait(&cond_in, &lock_in);
        }
        char *message = (char *)List_remove(list_in);
        // printf("[print] %s\n", message);
        // fprintf(stdout, "%s\n", message);
        puts(message);
        free(message);
        pthread_mutex_unlock(&lock_in);
    }
    return NULL;
}

// function for initialization
void init(int argc, char **argv) {
    // if error in input arg, exit program
    if (argc != 4) {
        fprintf(stderr, "Usage: %s [my port number] [remote machine name] [remote port number]\n", *argv);
        exit(EXIT_FAILURE);
    }

    // spicify my_port, remote_host, remote_port from input arg
    remote_host = argv[2];
    my_port = atoi(argv[1]);
    remote_port = atoi(argv[3]);

    // initialize lists
    list_in = List_create();
    list_out = List_create();

    // initialize mutexes
    pthread_mutex_init(&lock_in, NULL);
    pthread_mutex_init(&lock_out, NULL);

    pthread_cond_init(&cond_in, NULL);
    pthread_cond_init(&cond_out, NULL);

    over = false;
}

// check if user wants to exit
// end all threads if true
void *check_over(void *arg) {
    while (true) {
        if (over) {
            for (int i=0; i<4; ++i) {
                pthread_cancel(tid[i]);
            }
            break;
        }
    }

    return NULL;
}

// create threads
void run() {
    pthread_t t;
    pthread_create(&t, NULL, check_over, NULL);

    pthread_create(tid+0, NULL, sender, NULL);
    pthread_create(tid+1, NULL, receiver, NULL);
    pthread_create(tid+2, NULL, keyboard, NULL);
    pthread_create(tid+3, NULL, print, NULL);

    for (int i=0; i<4; ++i) {
        pthread_join(tid[i], NULL);
    }
}

// free out memory used by list
// end all mutexes and conditional variables
void destroy() {
    List_free(list_in, free);
    List_free(list_out, free);
    pthread_mutex_destroy(&lock_in);
    pthread_mutex_destroy(&lock_out);
    pthread_cond_destroy(&cond_in);
    pthread_cond_destroy(&cond_out);
}

// main
int main(int argc, char **argv) {
    init(argc, argv);
    run();
    destroy();

    return 0;
}

