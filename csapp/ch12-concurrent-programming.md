Application-level concurrency 的作用：

* *Aceessing slow I/O devices*: 当应用需要等待 slow I/O device 时，通过并行，应用可以干其他事情。
* *Interacting with humans*：用户可能希望能同时干多件事情。每当用户进行一个操作时，会创建一条新的 concurrent logical flow 来执行操作。
* *Reducing latency by deferring work*：在进行某一项任务之前，任务中的某一个操作可能很慢（例如 `free`），
延迟执行这个操作能减少任务完成时间。比如，延迟 `free` 的执行，到任务完成时，再一起来执行 `free` 操作。
* *Servicing multiple newtork clients*
* *Computing in parallel on multi-core machines*

三种实现并行的方式：

* *进程*：独立的地址空间，通过 *interprocess communication* 来通信
* *I/O multiplexing*
* *Threads*


# 12.1 Concurrent Programming wit Process
当 `fork` 一个子进程时，file tables 是共享的（为什么？见 10.8 以及 Figure 10.14），
虚拟地址空间是独立的。

**优点**: 独立的虚拟地址空间，一个进程不能对另外一个进程的虚拟内存进行写操作。

**缺点**: 难以共享信息，必须通过 IPC (interprocess communications) 来进行通讯（memory map 也可以吧？）；
创建进程的开销比较大。


Unix IPC: pipes, FIFOs, System V shared memory, System V semahpores。举个例子，可以利用 socket 接口来通信。

# 12.2 Concurrent Programming with I/O Multiplexing
当程序需要对多个 I/O 设备做出响应时，
可以使用 *I/O multiplexing* 技术。

比如，可以使用 `select` 函数来等多个 I/O 事件。
等待过程中进程被挂起(suspend)，当给定的 I/O 事件发生时，再唤醒程序。

例子：`select` 函数来 wait for reading。

`select` 函数原型：P977

`select` 函数有一个参数 `fdset`，代表所有 *descriptor* 的集合。

当 `select` 函数返回时，`fdset` 会更改为 `ready_set`，表示所有 *ready for reading* 的 descriptor。
(这个奇怪的设计导致每次调用 `select` 函数时都需要重新指定 `fdset`)

例子：Figure 12.6, echo server。

## 12.2.1 A Concurrent Event-Driven Server Based on I/O Multiplexing
I/O multiplexing 可以被用来实现 concurrent *event-driven* 程序。

这种程序可以用一个 *state machine* 来表示：

*state*：比如等待某一个 descriptor to be ready for reading

*input events*: 当 state 遇到某一个事件时，可能跳转到另外的 state

*transitions*: state 与 state 之间的转换


例子： Figure 12.8 - 12.10。

## 12.2.2 Pros and Cons of I/O Multiplexing
**优点**:

1. 给程序员更大的控制权（来控制并行的方式）。比如想先对某个 clients 进行服务。
2. 在一个进程中运行，方便共享数据；方便 debug；不需要 context switch

**缺点**:
1. coding complexity。比如，怎么处理某一个客户端在读取大量的数据，其它客户端等待的情况。

2. 不能充分利用 multi-core 处理器。

# 12.3 Concurrent Programming with Threads
Thread is a logical flow that runs in the context of a process.

每一个 thread 有自己的 *thread context*: thread ID, stack, stack pointer, program counter, general-purpose registers, condition codes。
(反正就是自己的，id, stack 和寄存器)
但是一个进程里所有的 thread 都共享 virutal address space。

## 12.3.1 Thread Execution Model
## 12.3.2 Posix Threads
## 12.3.3 Creating Threads
## 12.3.4 Terminating Threads
## 12.3.5 Reaping Terminated Threads
## 12.3.6 Detaching Threads
## 12.3.7 Initializing Threads
## 12.3.8 A concurrent Server Based on Threads
# 12.4 Shared Variables in Threaded Programs
## 12.4.1 Threads Memory Model
## 12.4.2 Maping Variables to Memory
## 12.4.3 Shared Variables
# 12.5 Synchronizing Threads with Semaphores
## 12.5.1 Progress Graphs
## 12.5.2 Semaphores
## 12.5.3 Using Semaphores for Mutual Exclusion
## 12.5.4 Using Semaphores to Schedule Shared Resources
### Producer-Consumer Problem
### Readers-Writers Problem
## 12.5.5 Putting It Together: A Concurrent Server Based on Prethreading
# 12.6 Using Threading for Parallelism
### Characterizing the Performance of Parallel Programs
# 12.7 Other Concurrency Issues
## 12.7.1 Thread Safety
## 12.7.2 Reentrancy
## 12.7.3 Using Existing Library Functions in Threaded Programs
## 12.7.4 Races
## 12.7.5 Deadlocks
