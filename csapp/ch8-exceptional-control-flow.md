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

### 8.4.6 Using `fork` and `execve` to Run Programs
Unix shell 和 web server 会频繁地调用 `fork` 和 `execve` 函数。

Figure 8.23, 8.24, 8.25: 一个简单的 shell 程序。
