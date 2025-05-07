# vBBS - A modern BBS for retrocomputing platforms

Please note: the code is not yet usable. This is a work in progress.

These are the primary goals of this BBS software:
1. Support for the following platforms:
   1. UNIX / POSIX (4.4 BSD, Linux, etc.)
   2. Mac OS System 7
   3. DOS
2. Support for the following connection methods:
   1. Hayes compatible modems
   2. Direct serial connection
   3. Telnet (where possible)
4. Support for the following features:
   1. ANSI color and screen control for supported clients
   2. Direct mail between users and via FidoNet
   3. Threaded bulletin boards and Echomail areas
   4. File upload/download areas with XModem and ZModem support
   5. Multiple concurrent users (where possible)
   6. DOOR game support 
5. All code written in ANSI C ('89) whenever possible, using no outside libraries to improve portability. 
6. Platform specific code should be minimized and the system should run with degraded features if a given platform doesn't support those features.

Initially I am focusing my development on UNIX. Once I have a working system I plan to port the code to run on Mac OS System 7 and then finally to DOS. I hope to support other platforms such as Amiga OS and CP/M if possible. 
