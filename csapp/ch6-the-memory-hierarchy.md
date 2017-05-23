# 6.1 Storage Technologies
## 6.1.1 Random Access Memory
分为 *Static RAM* 和 *Dynamic RAM* (Figure 6.2)

* SRAM 更快，更贵，用作 cache
* DRAM 用作 main memory 或者 frame buffer for a graphics system。

PS: 感觉这一节的内容不太会被面试官用到，所以先不看了，后续补上。

# 6.2 Locality
*Temporal locality*: a memory location that is referenced once is likely to be referenced again multiple times in the near future.

*Spatial locality*: if a memory location is referenced once, then the program is likely to reference a nearby memory location in the near future.

## 6.2.1 Locality of References to Program Data
Figure 6.18 & 6.19 (good locality and poor locality when visiting a two-dimension array).

## 6.2.2 Locality of Instruction Fetches
指令从内存中读取，所以指令的读取最好也能遵循局部性原理。

## 6.2.3 Summary of Locality

* 重复地使用同一个变量的程序有良好的 temporal locality
* stride-k reference pattern (访问变量顺序 a[0], a[k], a[2k]，即间隔 k) 的间隔 k 越小，spatial locality 越好
* 从 instruction fetch 的方面来讲，循环具备良好的 temporal 和 spatial locality；loop body 越小，loop 次数越多，locality 越好

# 6.3 The Memory Hierarchy
Figure 6.21: the memory hierarchy

## 6.3.1 Caching in the Memory Hierarchy
这一级的存储设备是下一级存储设备的 cashe。

PS: cache 念 cash

数据经常在 level k 和 level k+1 的 memory 之间来回拷贝。
拷贝的单位是 block，并且 block size 往往是固定的。
比如 Figure 6.21 中，L1 和 L0 使用 word-size blocks，

### Cache Hits
当应用程序需要从 level k+1 读取数据，而这个数据恰好在 level k 的 memory 上时，称为 cache hit。

### Cache Misses
与上面相反，要读取的数据不再 level k，称为 cache miss。

Cache miss 发生时，level k 的 cache 需要从 level k+1 读取包含这个数据的 block 到 level k cache 中。

当 level k cache 满时，需要替换掉一个 block，挑选 victim block 的方法： random, least recently used.

### Kinds of Cache Misses

* *Compulsory miss* or *Cold misses*: 一开始，level k cache 没有任何数据时
* *Conflict miss*: 虽然有足够大的 cache 能装下所有被访问的数据，但是由于这些数据需要存放在 cache 中的某几个位置 (原因如下)，发生了 conflict。
* *Capacity miss*: 要访问的数据 (working set) 大于 cache 的大小

Cache miss 发生的时候，level k+1 中的 block 应该放在 level k 中的哪个地方呢？
一种方法是 level k 中的任何一个位置都可以放，比如 main memory。

但是，高速 memory 往往在硬件中实现，且速度需求很高，
使用这种方法时，很难对 block 进行定位。
对于这种 memory，level k+1 的 block 往往只能放在 level k 的某几个位置。

### Cache Management
包含：

* Partition the cache storage into blocks
* Transfer blocks between difference levels
* Decide when there are hits and misses and deal with them

Cache 管理由谁来做：

* Register: 由编译程序决定
* L1, L2, L3: 硬件
* DRAM main memory: 操作系统和 CPU 中的地址转换硬件

## 6.3.2 Summary of Memory Hierarchy Concepts

* 利用 temporal locality: 同一个数据被多次重复使用，会比较快，因为数据已经在 cache 中了
* 利用 spatial locality: 一个 block 可能包含多个数据，把一个 block 拷贝进 cache 之后，访问 block 里面的其他数据也会更快
