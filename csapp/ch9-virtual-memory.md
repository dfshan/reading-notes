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
