Thabani Chibanda

Files implemented by me:

MAIN:                       main.c
SHELL:                      myShell.c, myShell.c
MEMORY MANAGEMENT:          myMem.c, myMem.h
I/O & FILE SYSTEM:          myIO.c, myIO.h
SUPERVISOR:                 mySVC.c, mySVC.h
SCHEDULER:                  myScheduler.c, myScheduler.h
DEBUG:                      dbg.c

MAIN

 ** Fixed malloc assignment problems. Also fixed the memory leak that was occuring from fill_argv(). One warning on unused variable that can be ignored.

 Main merely organizes the running of the program into a simple construct. Main handles output to the user, the parsing of user input, and then passes the command and it's argument to the shell module.


SHELL

 This file contains the shell commands that can be issued by the user. When a command is called, the necessary system call is made by the kernel to execute that command from the user. There are also 5 test commands for playing with the push buttons/LEDs to test if they work.

 The functionality of each function can will described by typing 'help' on it's own to get all the functions, or followed by the name of a specific command to get just that command run.

    REFERENCES: The part of my free() function that I now use to convert string to hex was put together with a bit of help from a reference online



MEMORY MANAGEMENT

 ** Fixed memory management implementation so that it now allocates the MB first and then works within this block. Improved my coalesce technique so now it doesn't make multiple walks through the list. Memory Map now just displays pid, size, and address. Also added the 8byte alignment I hadn't done correctly before. And the .h file now should correctly work to get the shell to call my functions in place of system ones.

 My memory management program uses two explicit linked lists for tracking the available spaces and the processes in use. I also chose to implement the first fit algorithm for finding a new memory block to allocate. The biggest reason for this was because it's the fastest implementation when using explicit linked lists(which are a bit time consuming to traverse). I also chose to split only significantly large free blocks upon allocation, so worst-fit would've wasted quite a bit of memory since it normally requires not ever split blocks. On the other hand, first fit also won't be forced to split many blocks due to the nature of the operating systems implementation, but it will split when necessary and keep the most amount of space free as possible. Lastly, I've chosen to implement coalescing on every chance possible, so reordering the avaliable spaces LL made the most sense. In turn, this made best fit a worse option then first fit because it seems as though first fit causes a lot more spaces that are near each other to be allocated(since free spaces are always taken from as high on the free spaces LL as possible). This means more coalescing yet a lot less traversals through the list for reording and searching.

    REFERENCES: I referenced to two pdf's for implementing malloc. They're called "A Malloc Tutorial" by Marwan Bruelle and "Optimizing Free & Malloc". They have a bit of code on the page, but they're very bare and simple allocation implementations. I used them to give me an idea on where to go with this, but I'm putting it here to be sure to list any references I used at all.

    Link Here:  http://www.inf.udec.cl/~leo/Malloc_tutorial.pdf
                /https://www.cs.princeton.edu/courses/archive/fall06/cos217/lectures/15Memory-2x2.pdf

    How to use it:

        malloc 'x' - command takes one non-negative integer 'x' and allocates memory of that size. Outputs address of memory allocated

        free '0x...' - takes a hex address that was malloc'd and frees. Notifies the user if address is successfuly freed

        mmap - shows all allocated memory(note: show memory from shell itself as well)

I ran a simple allocation test in main of myMem.c that output this to the memory map:

    1) Before malloc

    TOTAL FREE SPACE: 1024
    FINISH MAP

    2) After malloc

    TOTAL FREE SPACE: 904
    Proc: 16 : 0 : 0x7a614ca0
    Proc: 16 : 0 : 0x7a614cb0
    Proc: 16 : 0 : 0x7a614cc0
    Proc: 16 : 0 : 0x7a614cd0
    Proc: 16 : 0 : 0x7a614ce0
    Proc: 16 : 0 : 0x7a614cf0
    Proc: 24 : 0 : 0x7a614d00
    FINISH MAP

    3) After free

    TOTAL FREE SPACE: 1016
    Proc: 16 : 0 : 0x7a614cb0
    Proc: 16 : 0 : 0x7a614cf0
    Proc: 24 : 0 : 0x7a614d00
    Space: 16 : 0 : 0x7a614ca0
    Space: 48 : 0 : 0x7a614cc0
    FINISH MAP

    4) After more malloc

    TOTAL FREE SPACE: 952
    Proc: 16 : 0 : 0x7a614cb0
    Proc: 16 : 0 : 0x7a614cf0
    Proc: 24 : 0 : 0x7a614d00
    Proc: 16 : 0 : 0x7a614ca0
    Proc: 16 : 0 : 0x7a614cc0
    Proc: 16 : 0 : 0x7a614cd0
    Proc: 16 : 0 : 0x7a614ce0
    FINISH MAP

    5) After more free

    TOTAL FREE SPACE: 984
    Proc: 16 : 0 : 0x7a614cb0
    Proc: 16 : 0 : 0x7a614cf0
    Proc: 24 : 0 : 0x7a614d00
    Proc: 16 : 0 : 0x7a614ca0
    Proc: 16 : 0 : 0x7a614cd0
    Space: 16 : 0 : 0x7a614ce0
    Space: 16 : 0 : 0x7a614cc0
    FINISH MAP

 My '.h' file does one thing worth explicitly noting, it acutally just disables system malloc & free and redifines them for any files using myMem in the macros. This allows me to call 'malloc' and 'free' but have it come from my program. The rest in there is just standard function and variable declarations.



INPUT/OUTPUT & FILE SYSTEM

 ** Changed my implementation a bit by making it so everything is treated as a file. Now a few functions have become much simpler.

 My implementation of this system is fairly simple. Had a struct stream that pointed to the device functions and data. Device functions took care of manipulating most of the data and return results. Files and devices are created/opened/closed by the user. One important thing to note is that when writing to files, the kernel only allocates space when necessary. This seemed to be the best decision due to how the memory management is implemented.

    How to use it:

      "create file": takes a file name. (e.g "create text")
            - Filename only accepts numbers, letters, underscore, dash, and one extension. Can have any extension user would like.
            - Can't create two files of the same name.
            - Can only open 250 files

      "delete file": takes a file name. (e.g "delete text")
            - delete's that file from memory if it exists and isn't open.

      "open file mode":takes a file/device name to open and a mode. Returns a descriptor for issuing other commands to the stream. (e.g. "open f/text r")
            - Files must be written in the format "f/filename" and devices in the format "d/devicename"
            - Modes are 'r'(read), 'w'(write), and 'rw'(read and write).
            - All devices have exclusive access for reading and writing
            - File writers have exclusive access, and can't open file being read
            - An opened file places cursor at the beginning, so writing will overwrite data

      "close fd": takes a file/device descriptor to a specific stream (e.g "close f1")

      "read fd": takes a file/device descriptor and reads the next character or lets you know that you're at the EOF (e.g "read f1")

      "write fd char": takes a file/device descriptor to a specific stream along with a single character to write. Only returns failure (e.g "write f1 c")
            - Devices only take 0 to turn them on and a 1 for off(mostly applies to LED)

      "seek fd integer": takes a file descriptor and moves cursor to spot specified by user. Can also enter '-1' to be taken directly to the EOF, this can avoid overwrite on newly opened file.

      "ls" - lists all the files user has created

EXAMPLE:
(a bit long but just shows expected responses to commands)

    Thabani$ ./shell

    $ create thabani

    $ create farai

    $ create $#ads12-
    ERR: Could not create file.

    $ create thabani
    ERR: Could not create file.

    $ create chibanda12_.txt

    $ ls
    thabani     farai     chibanda12_.txt

    $ open f/thabani w
    File opened. file descriptor: f0

    $ open f/farai r
    File opened. file descriptor: f1

    $ write f0 a

    $ write f1 a
    ERR: File in use.

    $ read f1
    ERR: Read failed. End of file.

    $ write f0 b

    $ close f0
    File closed.

    $ open f/chibanda12_.txt rw
    File opened. file descriptor: f0

    $ open f/thabani r
    File opened. file descriptor: f2

    $ read f2
    output: a

    $ read f2
    output: b

    $ write f0 a

    $ write f0 b

    $ read f0
    ERR: Read failed. End of file.

    $ seek f0 0
    New File Positon: 0

    $ read f0
    output: a

    $ read f0
    output: b

    $ close f1
    File closed.

    $ delete f2
    ERR: File or device not found.

    $ delete thabani
    ERR: File in use.

    $ close f2
    File closed.

    $ delete thabani

    $ ls
    farai     chibanda12_.txt

    $ open d/orange rw
    Device opened. device descriptor: d2

    $ read d2
    Device is off.

    $ write d2 1

    $ read d2
    Device is on/active.

    $ open d/orange w
    ERR: Can't open file.

    $ close d2
    Device closed.

    $ exit



SUPERVISOR CALLS

 My svc file creates supervisor call instructions for my create, delete, open, close, get, put, list, get_desc, malloc, free, and memory map functions from 'myIO' and 'myMem'. I added them to my shell by just setting macros definitions to replace those calls from my myIO and myMem directly.

 Each SVC function essentially calls the corresponding function in Supervisor Mode. Now the control of all processes is running is by the kernel, or "supervisor". Each function is called, has it's parameters loaded into the registers and run.



SCHEDULER/PROCESS HANDLING(FINAL PROJECT):

 This file implements a scheduler into the operating system. First it contains functions for creating a process, and gives each one an appropriate process id. There are also system calls for blocking a process(either the current running process, or by pid for the supervisor), waiting on a process to terminate, and killing a process.

 The scheduler is a fairly basic multilevel queue. It is uses 3 separate queues to hold different priority levels, but each queue is first in first out. So highest priority processes are in the highest priority, and processes in the next queue are not scheduled to run until the higher priority queue is empty.

 Assembly is used to load and unload a new processes into the stack frame. Each process has it's arguments loaded into the proper registers, and metadata stored into the other registers. If a process is interrupted and it's not finished it's stack frame is saved by the scheduler before loading in a process.

    REFERENCES: I used a few references for building this schedule, they're pdf files and included in the project folder and marked.


dbg.h - This file you can ignore. It's a set of debugging macros I put together. Got the idea from the textbook "Learn C: The Hard Way"
