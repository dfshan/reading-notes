第一段阐述了 control flow 的概念。

# 8.1 Exceptions
Figure 8.1: exception， 一个程序在执行指令期间，发生了一个 event，
然后更具 exception table，CPU 会执行 exception handler 程序。
当 exception 程序执行完之后，有三种情况，
一是返回原来的位置执行；二是到原来的位置的下一条指令执行；三是退出原来的程序。
(是不是感觉和中断处理有点像？)

## 8.1.1 Exception Handling
每一个 exception 都有一个唯一的 *exception number*。谁来分配？处理器或者操作系统内核。

当系统启动时，操作系统会初始化一个叫做 *exception table* 的 jump table (Figure 8.2)，table 中的第 k 项存储的是 exception \\(k\\) 的 exception handler 的地址。

当系统执行时，处理器会检测并确定 exception number \\(k\\)，然后触发 exception，通过什么方式触发？
根据 exception table 来 making an indirect procedure call to the exception handler。
Figure 8.3

Exception 和函数调用的区别：

* 处理器会把程序的返回地址 (return address) 压到栈里面去，但是返回地址有可能是当前的指令，也有可能是下一条指令
* 处理器可能会把一些额外的处理器状态压到栈里面，比如 EFLAGS register
* 当控制权从用户程序转移到 kernel 时，上面的这些变量会被压入 kernel stack，而不是 user stack
* Exception handler 是运行在内核态的

## 8.1.2 Classes of Exceptions
### Interrupts
Figure 8.5 所示。
Interrupts occur *asynchronously* as a result of signals from I/O devices that are external to the processor.
(它指的是硬终端吧)

I/O 通过 interrupt pin 来终端处理器，并把中断号发送到 system bus 上。
当处理完当前指令之后，处理器根据中断号，调用相应的 interrupt handler。
Interrupt handler 处理完了以后，返回下一条指令执行。

### Traps and System Calls
Figure 8.6

### Faults
比如 page fault。
Handler 尝试 correct the error condition。如果 correct 成功，则返回当前指令重新执行，否则，通过 `abort` routine 结束程序。

Figure 8.7

### Aborts
Aborts result from unrecoverable fatal erros，通常是硬件错误。
 Handler 直接通过 `abort` routine 结束程序。

Figure 8.8.

## 8.1.3 Exceptions in Linux/x86-64 Systems
总共有 256 种 exception types。0-31 由 Intel architecture 定义，32-255 由操作系统定义，是 interrupts and traps.

### Linux/x86-64 Faults and Aborts

* Divide error: 除 0 或者除法的结果太大，Unix 不会尝试 recover，而直接结束，并 report "Floating exceptions"
* General protection fault: 比如访问虚拟内存中未定义的 area, 或者写入只读内存，Linux 同样不尝试 recover，并 report "Segmentation faluts"
* Page fault
* Machine check: as a result of fatal hardware error

### Linux/x86-64 System Calls
Figure 8.10

# 8.2 Processes
Process context: program's code and data, stack, general-purpose registers, program counter, environment variables, open file descriptors

## 8.2.1 Logical Control Flow
Figure 8.12: 3 个进程有各自的 logical control flow，轮流使用 CPU。

## 8.2.2 Concurrent Flows
两条 logical flow 的执行时间相互重叠，每一条 logical flow 称为 concurrent flow.

Parallel flow: 两条 flow 分别并行地在两个处理器上执行。

## 8.2.3 Private Address Space
Figure 8.13: x86-64 Linux process 的虚拟地址空间。

## 8.2.4 User and Kernel Modes
通过用户态和内核态，来区分权限：
区分哪些指令程序可以执行；哪些虚拟内存空间程序可以访问。

在 Linux 中，user mode processes 可以通过 `/proc` 文件系统读取 kernel data structures.

Linux 2.6 以后，还可以通过 `/sys` 文件系统来获取一些 low-level 的系统信息，比如 system buses and devices.

## 8.2.5 Context Switches
一个进程的 context 包括 general-purpose registers, floating-point registers, program counter, user's stack, status registers, kernel's stack, kernel data structures such as *page table*, *process table*, *file table*.

# 8.3 System Call Error Handling
当 Unix system-level functions 出现错误时，它们通常会返回 -1，并且设置全局变量 `errno`。 P737 的程序展示了如何进行 error checing.

# 8.4 Process Control
## 8.4.1 Obtaining Process IDs
`getpid` 函数返回当前进程的 PID.

`getppid` 函数返回 parent process 的 PID.

## 8.4.2 Creating and Terminating Processes
一个进程有三个状态 (from a programmer's perspective):

* Running: 正在 CPU 上执行或者等待执行
* Stopped: 不会被调度到，直到收到 SIGCONT 信号。
* Terminated: 永远都不会被执行。可能是由于 1) receive a signal whose default action is to terminate the process, 2) returning from the main routine, 3) calling the `exit` function.

Parent process 可以通过 `fork` 函数创建子进程。

`fork` 函数还有以下几点需要注意：

* Call once, return twice: once to parent (return PID of child) and once to newly created child (return 0).
* Concurrent execution
* Duplicate but separate address space:
子进程的 user-level virtual address space 和父进程一样，
包括 code, data segments, heap, shared libraries, and user stack.
* Shared files:
The child also gets identical copies of any of the parent's open file descriptors, which means the child can read and write any files that were open in the parent when it called `fork`.
(Figure 10.14)

## 8.4.3 Reaping Child Processes
当一个进程结束时，内核不会马上把它从系统中移除，直到这个进程被 parent *reaped*.
一个已经 terminated 但是还未被 reaped 的进程叫做 *zombie*. Zombies 虽然不会再运行，但是会占用系统资源。

当 parent 进程结束时，其创建的子进程就成了孤儿，这些进程交给 `init` 进程来抚养。

`init` process, which has a PID of 1, is created by the kernel during system start-up, never terminates, and is the *ancestor of every process*.

如果 parent process 结束时没有 reap child processes，那么 `init` 进程回来 reap 它们。

父进程通过 `waitpid` 函数来等待子进程结束并进行 reap。

### Determining the Members of the Wait Set
通过 `pid` 参数

### Modifying the Default Behavior
`options` 参数的使用

### Checking the Exit Status of a Reaped Child
通过 `statusp` 参数

### Error Conditions
通过返回值和 `errno`

### The `wait` Function
`waitpid` 的简化版本

### Examples of Using `waitpid`

## 8.4.4 Putting Processes to Sleep
The `sleep` function suspends a process for a specified period of time

`pause` function: puts the calling function to sleep until a signal is received by the process.

## 8.4.5 Loading and Running Programs
`execve` function: 在当前 process 的 context 下 loads and runs new program

`execve` is called once and never returns. (只有发生了错误时才会 return，比如 unable to find `filename`)

## 8.4.6 Using `fork` and `execve` to Run Programs
Unix shell 和 web server 会频繁地调用 `fork` 和 `execve` 函数。

Figure 8.23, 8.24, 8.25: 一个简单的 shell 程序。

# 8.5 Signals
Figure 8.26 Linux Signals

## 8.5.1 Signal Terminology
*Sending a signal*: Kernel sends a signal to a destination process by updating some state in the context of the destination process.

*Receiving a signal*: A destination process receives a signal when it is forced by the kernel to react in some way to the delivery of the signal. 如 Figure 8.27 所示。

任何时刻，最多只能有一个某种特定类型的 signal

## 8.5.2 Sending Signals
### Process Groups
任何一个进程都属于一个 process group。Process group ID 可以通过 `getpgrp` 函数获得。

默认情况下，子进程和父进程的 group 是一样的，但是可以通过 `setpgid` 函数来更改本进程所属的 group。

### Sending Signals with the `/bin/kill` Program
可以通过 `kill -<signal> <process id>` 命令来给进程发送 signal。

可以通过 `kill -<signal> -<process id>` 命令来 process group 发送 signal。

### Sending Signals from the Keyboard
通过 `jobs` 查看 background jobs.

输入 `Ctrl+C` 会使 kernel 给所有的 foreground process group 发送 SIGINT 信号

输入 `Ctrl+Z` 会使 kernel 给所有的 foreground process group 发送 SIGTSTP 信号

### Sending Signals with the `kill` Function
一个进程可以通过 `kill` 函数来给另外一个进程发送信号

### Sending Signals with the `alarm` Function
`alarm` 也称为闹钟函数。
`alarm()` 用来设置信号 SIGALRM 在经过参数 seconds 指定的秒数后传送给目前的进程。
如果参数 seconds 为0，则之前设置的闹钟会被取消，并将剩下的时间返回。
要注意的是，一个进程只能有一个闹钟时间，如果在调用alarm之前已设置过闹钟时间，
则任何以前的闹钟时间都被新值所代替。

## 8.5.3 Receiving Signals
当前 kernel 把一个进程从内核态切换为用户态时，它会检查当前进程的 unblocked pending signals。

如果没有这样的 signal，那么从下一条指令开始执行。

如果有这样的 signal，那么选择一个 signal \\(k\\) (一般选择最小的 \\(k\\)) 执行。

进程收到 signal 之后，会进行一些操作。进程执行完操作以后，会从下一条指令开始执行。

Figure 8.26 列举了一些 signal 的 default action.

一个进程可以通过 `signal` 函数来更改 default action。

Note: SIGSTOP 和 SIGKILL 的 default action 不能被更改。

例子：Figure 8.30

Signal handler 可以被其它 handler 中断，如 Figure 8.31 所示。

## 8.5.4 Blocking and Unblocking Signals
*Implicit blocking mechanism*: 如果当前进程正在处理一个 signal，那么 kernel 会把其他同一种类型的 pending signal 给 block 掉

*Explicit blocking mechanism*: 可以使用 `sigprocmask` 来 block 和 unblock signals

## 8.5.5 Writing Signal Handlers
Writing signal handlers is difficult:

1. Handlers 和 main program 是并行运行的，所以 handler 和 handler，
以及 handler 和 main program 可能会发生冲突。
比如同时对全局变量的读写，可能导致不可预测的结果。
2. 何时以及如何接收 signal 常常是有悖常理的
3. 不同的 system 可能有不同的 signal-handling semantics

### Safe Signal Handling
一些建议：

1. Keep handlers as simple as possible

2. Call only async-signal-safe functions in your handlers.
什么叫 async-signal-safe? 就是这个函数可以被 safely called from a signal handler.
什么样的函数是 async-signal-safe? reentrant (Section 12.7.2), 或者不能被其它的 signal handler 中断。
Figure 8.33 列举了一些 system-level safe functions.

3. Save and restore `errno`.
Handler 和其它函数可能同时需要使用 `errno`，
为了避免冲突，最好是把 `errno` 存在一个局部变量中，函数返回时再重新 restore。

4. Protect accesses to shared global data structures by blocking all signals.
如果确实需要访问共享的数据结构，要先把其它所有的 signal blcok 掉，
否则在访问共享数据结构的同时，可能被其它 handler 中断，导致不可预测的结果。

5. Declare global variables with `volatile`.
如果 handler 中更新了全局变量，main program 可能不知道，因为这个全局变量可能在 cache 在寄存器中。
`volatile` 会使 complier 从内存中读取变量，而不是从 cache 中。
6. Declare flags with `sig_atomic_t`. 对 `sig_atomic_t` 类型的变量的读和写都是 atomic (uninterruptible) 的。

### Correct Signal Handling
Signals cannot be used to count the occurrence of events in other processes.
Existence of a pending signal merely indicates that *at least* one signal has arrived.
如果有多个相同类型的 signal 同时到来，那么进程可能只收到一个 signal.

为什么？进程是否收到 signal 通过 `pend` bit 来查看，`peng` bit 只有一位，指示是否收到，而不是收到多少个。

例子：Figrure 8.36: 每收到一个 SIGCHLD 信号，reap 一个子进程，但是发送三个 SIGCHLD 信号，可能只 reap 两个子进程。

### Portable Signal Handling
不同的系统可能有不同的 signal-handling semantics.

可以通过 `sigaction` 函数来指定 signal-handling semantics.
由于 `sigaction` 需要操作复杂的数据结构，所以比较笨重。
所以 Figure 8.38 提供了一个包装函数 (wrapper function)。

## 8.5.6 Synchronizing Flows to Avoid Nasty Concurrrency Bugs
Figure 8.39: 该程序可能由于 race 导致 unexpected result.
即 `deletejob` 可能在 `addjob` 之前被调用。

Figure 8.40: 消除 race 的一种方法。

## 8.5.7 Explicitly Waiting for Signals
有的 program 比如 shell, 在创建子进程之后，需要等待子进程结束，然后 reap 子进程。

Figure 8.42 展示了如何使用 `sigsuspend` 函数来实现这种功能。

# 8.6 Nonlocal Jumps
Nonlocal jumps is a user-level exceptional control flow,
which can transfer control directly from one function to another (w/o call-and-return).

有什么用？
1. immidiate return from a deeply nested function call. (例子：Figure 8.43)
2. 当收到一个 signal 时，执行另外一段代码，而不是从中断的地方开始执行。(例子：Figure 8.44)

`setjmp` 和 `longjmp` 函数:

`setjmp`: 进行设置，该函数返回时可能有两种情况:
一是正常情况，也就是该函数被调用时，正常返回，返回值为0。
不正常的地方在于，当下面的代码调用 `longjmp` 函数时，该函数也会返回，返回值由 `longjmp` 设置。
该函数会把 calling environment 存到 `env` buffer 里。


`longjmp`: 直接跳转到 `setjmp` 的位置。跳转之前从 `env` buffer 里恢复 calling environment.


# 8.7 Tools for Manipulating Processes

* `strace`: 跟踪进程中的系统调用
* `ps`, `top`
* `pmap`: Displays the memory map of a process
* `/proc`: Exports kernel data structures
