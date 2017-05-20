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
*Main thread*: 每一个进程开始执行的那个 thread。

*Peer thread*: 后续创建的 thread。

Figure 12.12: concurrent thread execution

Thread 的 context switch 开销更小，因为 virtual address space 是一样的，不需要 flush cache, TLB。thread 主要是重置 CPU 寄存器。

任何一个 peer thread 都可以 kill 任何一个其它的 peer thread。

## 12.3.2 Posix Threads
Figure 12.13 例子

## 12.3.3 Creating Threads
P988: `pthread_create` 函数原型

`pthread_self` 可以得到自己的 thread 的 `tid`。


## 12.3.4 Terminating Threads
两种方式：

1. 当 thread routine 返回时
2. 调用 `pthread_exit` 函数。当 main thread 调用这个函数时，它会等待所有的 peer thread 结束，然后结束 main thread 和整个进程。
3. 任何一个 peer thread 调用 `exit` 函数，都会结束整个进程
4. 其他的 peer thread 调用 `pthread_cancle` 来结束本线程


## 12.3.5 Reaping (收割) Terminated Threads
`pthread_join` 函数等待其他的线程结束，并且 *reaps* 结束的线程所用的内存资源。

## 12.3.6 Detaching Threads
任何一个线程，要么需要通过 `pthread_join` 函数来 reap memory resource，要么通过 `pthread_detach` 函数来 detach 线程。

Thread 有两种状态： `joinable` 和 `detached`。
Joinable thread can be reaped and killed by other threads。
Joinable thread 的内存资源 are not freed until it is reaped by another thread。

A detached thread cannot be reaped or killed by other threads. Its memory resources are *freed automatically* by the system when it terminates.

一开始 thread 是 joinable 的，可以通过 `pthread_detach` 函数将进程转换为 detached。

## 12.3.7 Initializing Threads
`pthread_once` 函数常用于初始化一些全局变量。

`pthread_once` 函数被调用时，它会调用 `init_routine` 函数 (`pthread_once` 的一个参数)。

在多线程编程环境下，`pthread_once` 调用会出现在多个线程中，
`init_routine` 函数仅执行一次，究竟在哪个线程中执行是不定的，是由内核调度来决定。


## 12.3.8 A concurrent Server Based on Threads
Figure 12.14

# 12.4 Shared Variables in Threaded Programs
## 12.4.1 Threads Memory Model
一个 thread 不能访问和更改另外一个 thread 的寄存器。
但是由于不同 thread 的 virutal address space 是一样的，
一个 thread 对虚拟内存中某一个位置的更改，其它 thread 都能感知得到。

由于每个 thread 都有自己的 stack，所以一般来讲不同 thread 是相互独立的。
但是，由于不同 thread 是共享 virtual address space 的，如果非要访问其它 thread 的 stack，也是能做到的。

## 12.4.2 Maping Variables to Memory
三种方式：

1. 全局变量：在虚拟内存中的 read/write area，整个进程只有一个 instance。
2. *Local automatic variables*: 在 thread 自己的 stack 中。
3. *Local static variables*: 可以在 thread routine 中被定义，但是也存在于 read/write area of virtual memory，
且虽然在不同的 thread routine 都可以定义，但是只有一个 instance。

## 12.4.3 Shared Variables
没什么干货

# 12.5 Synchronizing Threads with Semaphores
并行运行程序时，无法预测执行程序的顺序，导致并行程序在有共享变量时可能会出现 unexpected results。

Figure 12.16 例子：两个 thread 都对共享变量了 `cnt` 加 `niters` 次，但是得到的结果不一定是 `2 * niters`。

为什么？使用汇编代码来看 (Figure 12.17)，thread\\(_i\\) 的汇编代码分为 5 个部分：

* \\(H_i\\): 循环初始 Head
* \\(L_i\\): load `cnt` 到寄存器中
* \\(U_i\\): 更新寄存器的值（加 1）
* \\(S_i\\): 把寄存器的值存入内存
* \\(T_i\\): 循环结尾

显然，两个 thread 的这 5 部分汇编代码如果重叠执行，则会造成执行结果 unexpected。


## 12.5.1 Progress Graphs
使用坐标图来表示各个 thread 的代码执行轨迹。
坐标轴 k 表示 thread\\(_k\\) 的执行轨迹。
每一个点 \\((I_1, I_2, \cdots, I_n)\\) 表示 thread\\(_k\\) 完成了 instruction \\(I_k\\) 的执行。
每执行一条指令，就会把程序执行状态从一个点迁移到另外一个点。

在上面的例子中，\\((L_i, U_i, S_i)\\) 会对共享变量 `cnt` 进行操作，因此是一个 *critical section*。

在 progress graph 中，不同 thread 的 critical section 的 intersection 称为 *unsafe region* (Figure 12.21)。

所有的执行轨迹中，不进入 *unsafe region* 的轨迹是 *safe trajectory*。

## 12.5.2 Semaphores
*Semaphore* 是一个全局变量 s，它有一个非负的值。
该值能被两种操作改变：

* *P(s)* (test 操作): 如果 s 大于 0，则 s 减 1 并且 P 马上返回。否则，挂起 thread。当 s 变为大于 0 时，线程会被 V 操作唤醒。
* *V(s)* (increment 操作): 给 s 增加 1，唤醒由于 P 操作不成功而 block 掉的线程。当有多个被 block 掉的线程时，挑选一个线程来唤醒。

这两个操作必须是原子的，他们不能被 interrupt 掉。

Posix 所提供的 semaphore 操作函数 (P1002): `sem_init`, `sem_wait`, `sem_post`。

## 12.5.3 Using Semaphores for Mutual Exclusion
用 semaphore 来实现互斥：s 初始值为 1，用 P(s) 和 V(s) 包围 critical section。

这样一来，s 的值只能为 0 或者 1，所以这样的 semaphore 称为 *binary semaphore*，也称为 *mutex*。

P 操作称为 *locking*，V 操作称为 *unlocking*。

## 12.5.4 Using Semaphores to Schedule Shared Resources
调度对共享资源的访问
### Producer-Consumer Problem
Figure 12.23

缓存满时，producer 要等待 consumer 把缓存中的物品取走；当缓存为空时，consumer 要等 producer 把物品放入缓存内。

实现：Figure 12.24 & 12.25．需要三个　semaphores，
一个用于互斥访问 (`mutex`)，一个用于剩余可用缓存 (`slots`)，一个用于统计缓存中物品数目 (`items`)。

Producer:

	P(slots)
	P(mutex)
	...
	V(mutex)
	V(slots)

Consumer:

	P(items)
	P(mutex)
	...
	V(mutex)
	V(items)


### Readers-Writers Problem
Writers 对资源的反问是互斥的，但是可以有多个 thread 同时对资源读取。

*First readers-writers problem*：当同时有 reader 和 writer 等待时，优先调度 reader

*Second readers-writers problem*：当同时有 reader 和 writer 等待时，优先调度 writer

Figure 12.26: first readers-writers problem 的实现

Second readers-writers probelm:

	Semaphores: rmutex, wmutex, r, w;

Reader:

	P(r);
	P(rmutex);
	readcount ++;
	if (readcount == 1)
		P(w);
	V(rmutex);
	V(r)
	...
	P(rmutex);
	readcount --;
	if (readcount == 0)
		V(w);
	V(rmutex);


Writer:

	P(wmutex);
	writecount ++;
	if (writecount == 1)
		P(r);
	V(wmutex);
	P(w);
	...
	V(w);
	P(wmutex);
	writecount --;
	if (writecount == 0)
		V(r);
	V(wmutex);


## 12.5.5 Putting It Together: A Concurrent Server Based on Prethreading
*Prethreading*:
之前的 web server 是每次来一个 client 就创建一个 thread，
完成对 client 的服务之后就结束这个 thread，
这样需要频繁地创建和销毁 thread，比较低效。

因此，可以预先创建好多个 thread，然后使用 producer-consumer 模型：
来一个 client 表示生成一个产品；预先创建好的 thread 表示 consumer，需要拿到产品并服务。

代码见 Figure 12.28。


# 12.6 Using Threading for Parallelism
A parallel program is a concurrent program running on multple processors.

Synchronization overhead is expensive and should be avoided if possible.

### Characterizing the Performance of Parallel Programs
一些指标：

*Speedup* \\(S_p\\): 1 个 core 上运行的时间除以 p 个 core 上的运行时间:

$$S_p = \frac{T_1}{T_p}$$

*Efficiency*: speedup 除以 core 数：

$$E_p = \frac{S_p}{p}$$

Programs with high efficiency are spending less time synchronizing and communicating.

*Weak scaling*: 每个 processor 所处理的 amout of work 一定，随着 processor 数目的增加，程序所能处理的问题规模的增加数。

# 12.7 Other Concurrency Issues
## 12.7.1 Thread Safety
## 12.7.2 Reentrancy
## 12.7.3 Using Existing Library Functions in Threaded Programs
## 12.7.4 Races
## 12.7.5 Deadlocks

