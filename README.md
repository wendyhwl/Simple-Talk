# Simple-Talk

Terminal chat function built with pthreads, a kernel-level thread implementation for LINUX. 

## How it works
A simple "chat"-like facility that enables someone at one terminal (or Xterm) to communicate with someone at another terminal. To initiate an new talking session, two users will first agree on two things:
- the machine that each will be running on
- the port number each will use

## Usage

### Run the program
To compile and run the program, open two terminals and enter the following on both terminals:

```bash
make
./s-talk [my port number] [remote machine name] [remote port number]
```
For example, say that Fred and Barney want to talk. Fred is on machine "csil-cpu1" and will use port number 6060. Barney is on machine "csil-cpu3" and will use port number 6001.

To initiate a talk, Fred must type:
```bash
s-talk 6060 csil-cpu3 6001
```
And Barney must type:
```bash
s-talk 6001 csil-cpu1 6060.
```
### Erase All

To erase all built products:

```bash
make clean
```

## Author

Wendy Huang

Date: June 3, 2020

## Acknowledgements

- Assignment created by Profs Harinder Khangura and Brian Fraser at **Simon Fraser University**
- Using the precompiled instructorList.o (renamed it as list.o) provided by profs
- [SFU Academic Integrity](http://www.sfu.ca/students/academicintegrity.html)
