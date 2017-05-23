# 9.3 VM as a Tool for Caching
一开始，虚拟内存里面的所有数据都是存在 disk 中的（一般是一块连续的数据），
当应用程序需要读取数据时，数据会以块的形式被读入到物理内存中，
这个块叫做 *virtual pages*。
也就是说，物理内存和磁盘的交互是以 *virtual page* 为单位的。

PS: Virtual page 和 physical page (page frame) 一般大小一样。


因为 miss penalty 非常严重，

1. virtual page 的大小比较大，一般为 4KB-2MB。
2. DRAM cache 是 fully associative 的：任何一个虚拟页都能放在任何一个物理页当中。
3. OS 会使用非常复杂的页替换策略（比 SRAM Cache 的替换策略要复杂得多）
4. DRAM cache 一般使用 write-back，而不是 write-through
(所以强制关机有可能会导致数据丢失)

## 页表
页表是常驻于物理内存的一个表格，页表中的每一个表项记录着一个 page 的信息：

1. 有一个 valid 位来判断 virutal page 是否在物理内存中
2. valid=1，该页在物理内存中，该表项后面有一个地址，表示该页面的物理地址
3. valid-0，该页不在物理内存中，如果后面的地址是 null，则该页还未被分配，否则，后面的地址是虚拟页在磁盘中的地址。

## Page Faults (DRAM cache miss)
处理 Page Faults 的过程：

1. 触发 page fault exception
2. Fault exception handler 会选择一个 victim page
3. 如果该 victim page 被修改过，则把这个页 swap out (或者 page out) 到 disk 中
4. 把要读的页 swap in (page in) 到物理内存中。handler 返回
5. 重新启动地址转换的过程

*Demand Paging*: the strategy of waiting until the last moment to swap in a page

所有 modern systems 都使用 demand paging

## Locality (局部性原理？)
在程序执行期间，只有一小部分 active pages (working set)。
所以只有一开始，需要把这部分 working set 读入到主存的时候，才需要和 disk 交互。

*Trashing*: 当 working set 大小比主存大，或者应用程序没有 locality 时，需要频繁地进行 swaping，这种情况叫 trashing。


# 9.4 VM as a Tool for Memory Management
VM 能大大地简化内存管理。

不同的应用程序有不同的 page table 和不同的虚拟地址空间。

demand paging 和 separate virtual addres space 有非常大的好处:

* 简化 linking:
link 的时候，不同的应用程序可以使用一样的地址，而不用考虑 code 和 data 在物理内存的真实位置。
比如，在 Linux 中，代码段都是从 0x400000 开始的。

* 简化 loading:
便于 load executable and shared object files into memory.
比如要 load .text 和 .data sections of an object file
Linux loader 只要分配一下代码段和数据段，然后在 page table 中把相应的表项标记为 invalid。
最后把这些数据在 disk 中的地址填入到表项当中。

	Note: loading 的过程并没有真正把数据载入到主存中。

	Note: 把一块连续的虚拟页映射到一个文件中的一个任意的位置叫 *memory mapping*。
	Linux 中有一个 system call `mmap` 实现了这个功能。

* 简化 sharing:
有的时候不同的应用程序需要共享 code 和 data。
比如标准的库文件 `printf`。
使用 VM 来共享的方法很简单，只要将不同应用程序的 virual page 映射到同一块 physical page 就行。

* 简化内存分配:
如果需要(使用 malloc)分配一块连续的内存，只要分配若干块连续的 virtual page，
然后把这些 virutal page 映射为任意的 physical memory 即可。
也就是说，没有必要真正地去分配连续的物理内存。

# 9.5 VM as a Tool for Memory Protection
内存保护：

1. 不能修改 read-only 代码段
2. 不能修改 kernel 中的代码和数据
3. 不能修改和其它进程共享的虚拟页面
4. 不能修改其它应用程序的私有页面

通过 separate virtual address space，不同的应用程序对内存的访问可以隔离开来。

通过在 page table 中设置一些许可标志位，可以实现内存保护的其它功能，比如：

* SUP 位：表示这个页面需要 kernel (supervisor) 模式来访问
* READ 和 WRITE 位：顾名思义，这个页面能否被读和写

如果应用程序违反了这些许可标志位，则会触发 protection fault
Linux shell 会 report this exception as a **segmentation fault**。

# 9.6 Address Translation
CPU 中有一个 control register 叫做 *page table base register (PTBR)*.
PTBR 存储着当前 page table 的首地址。

虚拟地址（假设有 n 位）可以分成两个部分，
一个部分是 *virtual page offset (VPO)* （假设有 p 位），
另外一个部分是 (n-p) 位的 *virtual page number (VPN)*。
VPO 和 *physical page offset (PPO)* 的值是一样的。

**数据读取过程 (page hint)** (Figure 9.13(a)):

1. CPU 生成一个虚拟地址，并把虚拟地址交给 MMU
2. MMU 生成 PTE (page table entry) 的地址，并地址发送给 cache 或主存
3. Cache 或主存返回 PTE 给 MMU
4. MMU 根据 PTE 生成物理地址，并把根据物理地址，发送给 cache 或主存
5. Cache 或主存返回所要读取的数据

**数据读取过程 (page fault)** (Figure 9.13(b)):

1. CPU 生成一个虚拟地址，并把虚拟地址交给 MMU
2. MMU 生成 PTE (page table entry) 的地址，并地址发送给 cache 或主存
3. Cache 或主存返回 PTE 给 MMU
4. MMU 发现 PTE 中 valid bit 为0，因此 MMU 触发一个异常，调用 OS 中的 page fault exception handler。
5. Fault handler 选择主存中的一个 victime page，如果这个页被写过，则将该页 page out disk 中。
6. Fault handler 把要读取的页 page in，并更新 PTE
7. 重新启动地址转换过程。即 CPU 重新发送虚拟地址。
4. MMU 根据 PTE 生成物理地址，并把根据物理地址，发送给 cache 或主存
5. Cache 或主存返回所要读取的数据

## 9.6.1 Integrating Caches and VM
Figure 9.14: 在有 Cache 的情况下使用虚拟地址读取数据的过程

## 9.6.2 Speeding Up Address Translation with a TLB
**TLB (Translation Lookaside Buffer)** 是一个在 MMU 中的一块 small cache。

TLB 的输入是 VPN，输出是 PTE。
在 TLB 查找 PTE 的过程中，VPN 可以分成两个部分：*TLB tag (TLBT)* 和 *TLB index (TLBI)* (Figure 9.15)。
TLBI 用于选择 set，而 TLBT 用于匹配。
(Example: Figure 9.20(a))


**数据读取过程 (TLB hint)** (Figure 9.16(b)):

1. CPU 生成一个虚拟地址，并发送给 MMU
2. MMU 从 TLB 中读取 PTE
3. MMU 把虚拟地址转换为物理地址，并把物理地址发送给 cache/main memory
4. cache/main memory 返回要读取的数据


## 9.6.3 Multi-level Page Tables
需要在内存中的 page table 大小可能很大：
比如 32-bit address, 4KB pages (总共有 1MB pages)，4-byte PTE，则一张 page table 的大小为 4MB。
如果是 64-bit address 的话，page table 会更大。

为了减少内存占用，我们可以使用多级 page table。

例子(Figure 9.17)：一个两级 page table，第一级的 PTE 包含 1024 pages，指向 1024 个 PTE 的首地址。
当第一级的 PTE 所包含的 1024 pages 都还未被分配时，第一级的 PTE 地址为 null，否则，其指向 1024 个第二级 page table 的首地址。

**为什么多级页表能节省内存占用**:

1. 如果上一级的 PTE 为 null，那么下一级的 page table 就不需要存在。因为大部分的虚拟内存其实都是没有被分配的，所以很多 page table 都可以不存在于内存当中。

2. 只有第一级的 page table 需要常驻于内存当中，低级的 page table 只有要访问到的时候才需要被 page in 到主存当中。


一个多级页表的例子: Figure 9.18.

## 9.6.4 Putting It Together: End-to-End Address Translation
Figure 9.19: 虚拟地址和物理地址的格式
Figure 9.20: TLB, page table 和 cache

具体过程可以看 p823 页。


# 9.7 Case Study: The Intel Core i7/Linux Memory System
Intel Core i7:

* 48-bit virtual address, 52-bit physical address; compatibility mode: 32-bit virutal and physical address (虚拟地址空间比物理地址空间小)
* 两级 TLB，每个 TLB 是 4-way 的（也就是说，每次可以同时访问 4 个 entry）
* Figure 9.21 CPU 的结构
* Page size 可以设置为 4KB 或者 4MB，Linux 使用 4KB
* Four-level page table

## 9.7.1 Core i7 Address Translation
Figure 9.22.

四层 page table，在 Linux 中，所有被分配内存的页所对应的页表都是常驻于内存当中的。

*CR3* control register 中存放的是第一级 (L1) 页表的起始地址。

Figure 9.23 是第1-3级页表的表项的格式，Figure 9.24 是第四级页表表项的格式。

页表表项中的标志位：

* R/W: 是否只读
* U/S: 这个页面能否在用户态被访问
* XD: execute disable，是否禁止从该页面获取指令（用于减少 buffer overflow 带来的危害）

内核 fault handler 所使用的标志位:

* A bit (reference bit): 页面被访问
* D bit (dirty bit): 页面被写入数据

## 9.7.2 Linux Virutal Memory System
Figure 9.26: 一个 Linux 进程的虚拟内存结构

### Linux Virtual Memory Areas
Linux 把虚拟内存分为不同的 areas (或 segments)，例如 code segment, data segment, heap, shared library segment, user stack.

把虚拟内存分为不同 area 的好处是允许 virtual address space 有 gaps：
内核不会 keep track of virtual pages that do not exist，这些 pages 也不会占用资源（内存，disk，以及 kernel)。


Figure 9.27: 内核中用于 keep track of virtual memory areas 的数据结构：

`task_struct`: 每个进程都有的一个数据结构，用于存储进程的一些信息（PID, pointer to user stack, name of executable object file, program counter)

`mm_struct`: `task_struct` 里的一个变量，里面包含两个变量：

* `pgd`: 指向第一级 page table 的起始地址
* `mmap`: 指向 `vm_area_structs` 结构体链表，链表中的每一项都表示一个 area

### Linux Page Fault Exception Handling
当 MMU 触发 page fault 时，kernel 中的 page fault handler 会来处理这个异常：

1. 检查虚拟地址是否合法：检查该地址是否属于某一个 area，更具体的说，是否存在于某一个 `vm_area_structs` 中。
如果不存在，则触发 segmentation fault，进程终止
2. 检查访问是否符合权限：这个进程是否有权限对页面进行读或写或执行。如果不符合权限的要求，则触发 protection exception，终止进程
3. 选择 victim page，进行 swap out 和 swap in

# 9.8 Memory Mapping
Linux 使用 disk 中的 *object* 来初始化一块虚拟内存 area 的过程叫 *memory mapping*。

分为两类：

* Regular file in Linux file systems: 将虚拟内存映射为 disk file 中的一块连续数据。
文件被切分为 page size。这些数据在被使用到时才会被 swap in 内存。

* Anonymous file: 这个并不是真正的文件。文件中所有的数据都是0。
当 CPU 要访问这么一块 virutal page 时，首先会 swap out 脏页，然后把页面里的所有数据都置为0。
所以 swap in 的时候并没有和 disk 进行交互。这种 map 又称为 *demand-zero pages*。


## 9.8.1 Shared Object Revisited
通过 memory mapping 可以把不同进程的虚拟内存映射到同一个 object 上（比如 `printf`)。

Object 分为两种:

* shared object: 映射到 shared object 的虚拟内存 area 也成为 *shared area*，一个进程对 shared area 进行修改时，其它进程也能看得到。
* private object: 映射到 private object 的虚拟内存 area 称为 *private area*，一个进程对 private area 进行的修改，其它进程不能感知到。

例子：
Figure 9.29: 两个不同进程的虚拟内存被映射到同一个 shared object 中。

Figure 9.30: 两个不同进程的虚拟内存被映射到同一个 private object。
如果不进行写操作的话，不同进程的虚拟内存指向的是同一块物理内存。
Private area 的页是 read-only 的，更具体地说，这一块 area struct 会被标记为 *private copy-on-write*。
如果进程尝试进行写操作，会触发 protection exception，
此时 fault handler 会 copy 这一个物理页到另一个物理页中，然后重新进行写操作。

## 9.8.2 The `fork` Function Revisited
当一个进程创建新进程时，kernel 首先会创建进程的一些数据结构。
然后，为新进程创建和当前进程一样的 mm_struct 结构体。

接下来，就使用到了 private object：所有的页面都被标记为 read-only，并且两个进程中的每一个 area struct 都被标记为 copy-on-write。

可想而知，接下来只有当进程需要写页面时，才会创建新的页面。

## 9.8.3 The `execve` Function Revisited
当进程执行以下代码时:

	execve("a.out", NULL, NULL);

会进行以下操作：

1. 删除现有的 user areas
2. 对 private area 进行 map:

	* code 和 data areas 映射到 `a.out` 文件中的 `.text` 和 `.data` sections，标记为 copy-on-write
	* bss area, stack area, heap area 都是 demand zero

3. 对 shared area 进行 map:
dynamically link shared objects (e.g., standard C library libc.so) into the program,
and map into the shared region of the user's virtual address space.

4. 设置 program counter (PC)

## 9.8.4 User-Level Memory Mapping with the `mmap` Function
`mmap` 函数可以供 Linux 进程来进行 memory map

函数的说明见 P838.

# 9.9 Dynamic Memory Allocation
动态内存分配是在 *heap* area 中进行的。
Kernel 中的 `brk` (break) 变量指向 head 的头部。

Heap 由一些不同大小的 *blocks* 组成。
每一个 block 有可能是已经被 *allocated*，也有可能是 *free* 的。

有两种 *allocator*:

* **显式 allocator**: 需要显式地分配和释放 blocks。比如 C 中的 `malloc` 和 `free`。
* **隐式 allocator**: Allocator 需要检测出哪些 allocated block 不会再被用到了，然后自动地把这些 block 释放掉。这个过程也称为 *garbage collectors*。

## 9.9.1 The `malloc` and `free` Functions
In 32-bit mode (gcc -m32), `malloc` returns a block whose address is always a multiple of 8.

In 64-bit mode (gcc -m64), `malloc` returns a block whose address is always a multiple of 16.

`calloc`: 初始化动态内存

`realloc`: 改变之前被分配的 block 的大小

`malloc` 可以通过使用 `mmap` 和 `munmap` 来增大和减少 heap memory 的大小。
也可以使用 `sbrk` 函数。`sbrk` 函数会增加内核中的 `brk` 指针。

Figure 9.34: 一个简单的例子


## 9.9.2 Why Dynamic Memory Allocation ?
因为程序运行之前不知道该分配多少内存。

## 9.9.3 Allocator Requiremewnts and Goals

显示 allocator 需要满足下列需求：

1. 能处理任意顺序的 allocate 和 free 的请求:
Allocator 不能事先假定 allocate 和 free 请求的顺序
2. 马上对请求做出响应：
Allocator 不能对请求进行重新排序或延迟对请求的处理(比如 coalescing)以提高性能。
3. 只使用 heap:
任何 nonscalar 数据结构都要存储在 heap 当中。
4. 需要对 block 进行 align:
8 的倍数或 16 的倍数。
5. 不能更改被分配的 blocks:
Allcators 只能去修改 free blocks。

目标：
1. 最大化吞吐量：单位时间内能处理的请求数
2. 最大化内存利用率：
内存利用率可以用 *peak utilization* 来衡量，
即前 k 个请求中，被分配的最大大小除以第 k 个请求时 heap 的大小。(见 P845)

## 9.9.4 Fragmentation
两种 fragmentation: *internal fragmentation* 和 *external fragmentation*:

* *Internal fragmentation*: 被分配的 block 比实际需要的要大。理由：alignment 或 allocator 对最小大小有要求。
* *External fragmentation*: 两个 allocated blocks 之间有许多 free block，而每一个 free block 又不能满足要分配的内存的大小。

## 9.9.5 Implementation Issues

* Free block 的组织：怎么 keep track of free blocks
* Placement: 在分配内存时，怎么挑选一块 free block 分配给它
* Splitting: 当选择一块 free block 并进行分配时，怎么处理 free block 中剩下的部分。
* Coalescing: 当一块 block 被释放时，怎么处理？

## 9.9.6 Implicit Free Lists
每个 block 的格式如 Figure 9.35 所示。

每个 block 有一个头部，头部包含了两个信息：这个 block 的大小以及这个 block 是否被分配。

Figure 9.35 中，block size 必须时 8 的倍数，所以 block size 的最低3 位为 0，最低 3 位的最后一位用来表示该 block 是否被分配。

Figure 9.36 是整个 heap 的一个结构。heap 的最后一个 block 是一个 size 为 0 的 block，用来表示 heap 到此结束了。

这个 heap 结构可以通过头指针和每个 block size 来遍历所有的 block。
遍历过程中可以搜索 free block。
由于没有显示地把 free block 构造成为一个 list，所以称为 *implicit free lists*。

**优点**: 简单， **缺点**: 找到一块合适的 free block 需要 O(n) 的时间。

Alignment requirement and minimum block size:
假设 alignment 是 double-word，则每一块 block 的大小都必须是 9B 的倍数。另外，还可能需要 1 word 用来存储 header。

## 9.9.7 Placing Allocated Blocks
在受到内存分配请求时，allocator 需要选择一块空闲内存来分配，有三种策略：

1. *First fit*: 第一块被搜索到的大小足够大的 free block
2. *Next fit* (Donald Knuth): 从上一次搜索结束的地方开始搜索，然后搜索到第一块合适的 free block
3. *Best fit*: 所有的大小足够大的 free block 里面，找到最小的一块

## 9.9.8 Splitting Free Blocks
找到 free block 之后，有两种策略：一是把所有的 free block 都分配出去，优点是简单且快，缺点是有 internal fragmentation。
适用于要分配的大小与 free block 的大小相差不大。

二是把 free block 分为两个部分。

## 9.9.9 Getting Additional Heap Memory
当找不到足够大的能被分配的 free block 时 (即便把相邻的 free blocks 聚合起来也找不到)，
allocator 需要调用 `sbrk` 函数向内核申请新的内存，这一块内存作为一块新的 free block 插入到 free list 尾部。

## 9.9.10 Coalescing Free Blocks
如果有很多相邻的 free block，可能会导致 *false fragmentation* (Figure 9.38)，
即有很多 available free memory，但是都被分成了很小的不够用的 free blocks。

把这些 free blocks 聚合起来的过程叫 *coalescing*，分为 *immediate coalescing* (一个 block 被 free 了就 coalescing) 和 *deferred coalescing*。

*Immediate coalescing*: 很直接，但是频繁地 coalescing 和 split 可能会导致 trashing。

## 9.9.11 Coalescing with Boundary Tags
对于一个 free block 而言，它想 coalescing 后面的 block 容易(直接找到后面的 block check 一下是不是 free)，
但是想 coalescing 前面的 block 就不太容易了，问题类似于单向链表找 previous node。

*Boundary tags* (by Knuth)：在每一个 block 中加一个 *footer* (boundary tag)。
Footer 的值和 header 一模一样。这样单向链表就变成了双向链表。

## 9.9.12 Putting It Together: Implementing a Simple Allocator
有时间看看，可以加深对动态内存分配的理解。

### General Allocator Design
Figure 9.41

### Basic Constants and Macros for Manipulating the Free List
Figure 9.43

### Creating the Initial Free List
Figure 9.44 & 9.45

### Freeing and Coalescing Blocks
Figure 9.46

### Allocating Blocks
Figure 9.47

## 9.9.13 Explicit Free Lists
Implicit free list 的 block allocation time 是线性的，不适用于 general purpose allocator，适用于 heap blocks 数目较小的时候。

*Explicit free lists*: 把 free blocks 通过双向链表连接起来，如 Figure 9.48 所示。

在释放 block 的时候，需要把 free block 放入 free list 中，
有两种策略，一种是把 free block 放入链表头部，这样简单但是 memory utilization 可能不高;

另外一种是按 *address order* 来组织 free list，这样把 free block 插入 free list 可能需要线性的时间。

## 9.9.14 Segregated Free Lists
按不同的 free block 大小，把 free block 放入不同的 List 中存放。

这样分配请求时，就不需要遍历所有的 free blocks 了。

### Simple Segregated Storage
所有的 free list 仅包含相同大小的 free block。不切分 free block，即把整个 free block 分配出去。
如果某一个 free list 空了，直接向操作系统请求内存。

*优点*: 分配和释放内存都快，因为无需 splitting 和 coalescing。因此也不需要 footer，不需要双向链表。

*缺点*: internal fragmentaion and external fragmentation

### Segregated Fits (被 GNU `malloc` 使用)
Free lists 有不同的 size class，分别属于不同的 size 范围。

分配内存：从一个最小的并且合适的 size class 找起，如果找不到，到下一个更大的 size class 找。
找到了之后需要进行 split，并把剩下的 free block 插入到合适大小的 free list 中去。

释放内存：要 coalescing，并把聚合之后的 free list 插入到大小合适的 free list 中去。

### Buddy Systems
Segregated fits 的一种特殊情况，每个 size class 的大小是 a power of 2。

# 9.10 Garbage Collection
自动把程序中不会再被用到的 blocks 释放掉。

## 9.10.1 Garbage Collector Basics
*Reachability grash* (Figure 9.49)：
图中有两种节点 *root nodes* 和 *heap nodes*。
Root nodes 对应的是不在 heap 中的位置，但是有指针指向 heap 中的某个位置。
节点 p 指向节点 q 表示 p 中有指针指向 q 中的某一个位置。

节点 p 是可达的：有一条从 root 节点到 p 的路。

节点 p 不可达表示这一块 block 是 garbase，需要被搜集。

Garbage Collector 可以使用一个单独的 thread 去跑，也可以 on demand。

比如 (Figure 9.50)，当 `malloc` 函数被调用时且无法找到 free block 时，则进行垃圾搜集；
找到垃圾时，进行 `free` (所以 `free` 函数是 collector 调用的而不是 application)。

## 9.10.2 Mark&Sweep Garbage Collectors
两阶段：先把所有可达的节点 mark (*mark phase*)，然后把所有未被 mark 的 block 释放掉 (*sweep phase*)。

Figure 9.51: 算法伪代码。
Figure 9.52: 例子

## 9.10.3 Conservative Mark&Sweep for C Pgrograms
C 实现 Mark&Sweep 的困难:

1. 一块 memory 没有类型信息，所以无法判断数据的类型是否是指针
2. 不太容易判断指针指向的位置是否属于 heap。

为了解决第二个困难，可以把所有被分配的 block 组织成 balanced binary search tree (Figure 9.53)。
地址小的在左边，地址大的在右边。
通过搜索 binary tree，比较 address 和 block size 来判断地址是否在 heap 中。

Conservasive: 由于不能判断数据类型，可能把某些不是指针的数据误判为指针，
而这个数据所代表的地址又恰好指向某一块 heap block，就会导致错误地 mark 这一块 block。

# 9.11 Common Memory-Related Bugs in C Programs
作为一个程序员，
应该能深深地体会到 memory-related bugs (segmentatin fault) 对你的 "毒害"。

## 9.11.1 Dereferencing Bad Pointers
啥叫 dereference? 两个操作符：`&` 表示 reference, `*` 表示 dereference。

指针所指向的地址不合法，但是进行了 dereference --> segmentation fault。

比如，把某个非指针变量当成指针用：

	scanf("%d", val);

## 9.11.2 Reading Uninitialized Memory
顾名思义，直接看 P871 的例子吧。

## 9.11.3 Allowing Stack Buffer Overflows
P871 例子

## 9.11.4 Assuming That Pointers and the Objects They point to Are the same Size
P872 例子：line 5 should be `malloc(n * sizeof(int*))`

## 9.11.5 Making Off-by-One Errors
P872 例子: line 7 `i <=n ` instead of `i < n`

## 9.11.6 Referencing a Pointer Instead of the Object It Points To
P873 例子：line 6: `*size--` instead of `(*size)--`

## 9.11.7 Misunderstanding Pointer Arithmetic
对指针的运算是以指针所指向的 object size 为单位的。
例如 `int *p; p + 1`，实际上地址加了 `sizeof(int)`。

P873 例子

## 9.11.8 Referencing Nonexistent Variables
P874 例子

## 9.11.9 Referencing Data in Free Heap Blocks
指向已经被 free 掉的数据

P874 例子

## 9.11.10 Introducing Memory Leaks
忘记释放内存

P875 例子

# Appendix 1: Linux 内核中的寻址
Linux 进程的虚拟地址布局
![Virtual memory of a Linux process](static/vm-struct.jpg)

x86-32 系统中，内核虚拟地址分为两个部分：

1. LOWMEM: 0-896MB, 分为两个部分：
	1. ZONE_DMA: 0-16MB
	2. ZONE_NORMAL: 16MB-896MB (880MB)
2. HIGHMEM, 896MB-1024MB, 包含一下部分
	3. ZONE_HIGHMEM: 896MB-1024MB (128MB)

寻址方式分为两种情况，1). 物理内存大小小于 1GB; 2). 物理内存大小大于 1GB

对于第一种情况，内核虚拟地址空间能就能访问到所有的内存，所以虚拟地址通过线性映射转换为物理地址：

	physical address = virtual address - PAGE_OFFSET (PAGE_OFFSET = 0xc000 0000 in x86-32)

对于第二种情况，LOWMEM 的地址通过线性映射转换为物理地址，HIGHMEM 则不然。

HIGHMEM 的地址可能需要通过分配和释放的方式，来映射到整个物理内存中 (如使用 page table)

Note: 对于 64 位系统，所有的虚拟地址都通过线性映射转换为物理地址。

参考资料：

* [http://events.linuxfoundation.org/sites/events/files/slides/elc_2016_mem_0.pdf](http://events.linuxfoundation.org/sites/events/files/slides/elc_2016_mem_0.pdf)
* [http://blog.csdn.net/hanzy0823/article/details/11979407](http://blog.csdn.net/hanzy0823/article/details/11979407)
* [http://learnlinuxconcepts.blogspot.com/2014/02/linux-addressing.html](http://learnlinuxconcepts.blogspot.com/2014/02/linux-addressing.html)
