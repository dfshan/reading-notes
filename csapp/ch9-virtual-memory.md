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
