This package simulates the protocols of Chapter 3 of "Computer Networks 3/e" by Andrew S. Tanenbaum, published by Prentice Hall PTR, 1996. It was written by Andrew S. Tanenbaum and may be freely distributed.

The simulator is compiled by just typing 'make'. If you want to use gcc instead of cc, change the line `CC=cc` to `CC=gcc` in Makefile.

It is executed by giving a command line containing the simulation parameters.
The command line has six decimal parameters, as follows:

    sim  protocol  events  timeout  pct_loss  pct_cksum  debug_flags

where

-   `protocol` tells which protocol to run, e.g. 5
-   `events` tells how long to run the simulation
-   `timeout` gives the timeout interval in ticks
-   `pct_loss` gives the percentage of frames that are lost (0-99)
-   `pct_cksum` gives the percentage of arriving frames that are bad (0-99)
-   `debug_flags` enables various tracing flags:
1. frames sent
2. frames received
3. timeouts
4. periodic printout for use with long runs

For example

    sim 6 100000 40 20 10 3

will run protocol 6 for 100,000 events with a timeout interval of 40 ticks, a 20% packet loss rate, a 10% rate of checksum errors (of the 80% that get through), and will print a line for each frame sent or received. Because each peer process is represented by a different UNIX process, there is (quasi)parallel processing going on. This means that successive runs will not give the same results due to timing fluctuations.

---

Here are some notes on the simulator.

The simulator consists of two code files, sim.c and worker.c, along with a common header file, common.h. It also uses the five protocols p2.c through p6.c, which were supposed to be the same as in the book, but some small changes had to be made to make the simulation work. The protocols use the file protocol.h, which is Fig. 3-8 from the book.

The simulator uses three process:

    main:   controls the simulation
    M0:	machine 0 (sender for protocols 2 and 3)
    M1:	machine 1 (receiver for protocols 2 and 3)

The file sim.c contains the main program. It first parses the command line and stores the arguments in memory. Then it creates six pipes so the three parts can communicate pairwise. The file descriptors created are named as follows.

    M0 - M1 communication:
    w1, r1: M0 to M1 for frames
    w2, r2: M1 to M0 for frames

<br>

    Main - M0 communication:
    w3, r3: main to M0 for go-ahead
    w4, r4: M0 to main to signal readiness

<br>

    Main - M1 communication:
    w5, r5: main to M1 for go-ahead
    w6, r6: M1 to main to signal readiness

After the pipes have been created, the main program forks off two children, M0 and M1. Each of these then calls the appropriate protocol as a subroutine.
All the protocols are compiled into the binary, sim, so no exec is done.

Each protocol runs and does its own initialization. Eventually it calls `wait_for_event()` to get work. This routine, and all the others whose prototypes are in Fig. 3-8 are located in the file worker.c. `Wait_for_event()`
sets some counters, the reads any pending frames from the other worker, M0
or M1. This is done to get them out of the pipe, to prevents the pipes from
clogging. The frames read are stored in the array `queue[]`, and removed from
there as needed. The pointers inp and outp point to the first empty slot in
`queue[]` and the next frame to remove, respectively. Nframes keeps track of
the number of queued frames.

Once the input pipe is sucked dry, `wait_for_event()` sends a 4-byte
message to main to tell main that it is prepared to process an event.
At that point it waits for main to give it the go-ahead.

Main picks a worker to run and sends it the current time on file descriptors
w3 or w5. This is the go-ahead signal. The worker sets its own time to the
value read from the pipe, so the two workers remain synchronized in time.
Then it calls `pick_event()` to determine which event to return. The list of
potential events differs for each protocol simulated. The choices are made
in `pick_event()`, which checks to see what is possible. For example, if no
timers are running, or timers do not exist for the protocol being simulated,
the timeout event cannot be returned. If no frames are present in `queue[]`,
then a frame_arrival event is impossible, and so on.

Once the event has been returned, wait_for_event returns to the caller, one
of the protocol routines, which then executes. These routines can call the
library routines of Fig. 3-8, all of which are defined in the file worker.c.
They manage timers, write frames to the pipe, etc. The code is
straightforward and full of comments.

The main program is simple. It picks a process and gives it the go-ahead by
writing the time to its communication pipe as a 4-byte integer. That process
then checks to see if it is able to run. If it is, it returns the code OK.
If it cannot run now and no timers are pending, it returns the code NOTHING.
If both processes return NOTHING for DEADLOCK ticks in a row, a deadlock is
declared. DEADLOCK is set to 3 times the timeout interval, which is probably
overly conservative, but probably eliminates false deadlock announcements.
