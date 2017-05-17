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
