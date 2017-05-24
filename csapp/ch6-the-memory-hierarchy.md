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

# 6.4 Cache Memories
## 6.4.1 Generic Cache Memory Organization
Figure 6.25: cache 的结构：
总共有 \\(S=2^s\\) 个 sets，每个 set 有 \\(E\\) cache lines。
每个 Cache line 有一位 valid bit，表示这个 line 是否包含 meaningful information；
Cache line 还包含 \\(t\\) 位的 Tag 标志，然后是一个 \\(B=2^b\\) Bytes 的数据。

CPU 从 Cache 读取数据时，一个地址被切分成 Figure 6.25(b) 所示的几个部分。
首先根据 s 位的 set index 找到对应的 set，然后根据 tag bits 进行匹配，看是否有某一个 line 能匹配上 tag 并且 valid bit = 1.
最后，更具 block offset bits 来找到对应的数据。

思考：为什么前 t bits 是 tag ? 如果前几个 bits 是 set index，那么一段连续的数据会被映射到同一个 set，如果 \\(S > E\\)，则可能发生频繁的 conflict miss。

## 6.4.2 Direct-Mapped Caches
每个 set 仅包含一个 line 的 cache 叫做 *direct-mapped cache*。

### Set Selection in Direct-Mapped Caches
如 Figure 6.28 所示，从 address 中把 s 位的 set index 提取出来，用于选择 set。

### Line Matching in Direct-Mapped Caches
如 Figure 6.29 所示，如果下面两个条件都满足，则 Cache hit:

* valid bit = 1
* 从 address 中提取的 tag 和 cache line 中的 tag 吻合


### Word Selection in Direct-Mapped Caches
如 Figure 6.29 所示，提取出 byte offset，读出相应的数据

### Line Replacement in Direct-Mapped Caches
如果发生了 cache miss，则从下一级 memory 中把 block 读入，并且写入到 cache 相应的位置。

### Putting It Together: A Direct-Mapped Cache in Action
Just read the example

### Conflict Misses in Direct-Mapped Caches
Conflict misses in derect-mapped caches typically occur when program access arrays whose sizes are a power of 2.

例子 P622.

## 6.4.3 Set Associative Caches
Direct-mapped caches 的 conflict miss 比较严重，所以有了 set associative caches。

\\(E\\)-way set asssociative cache: \\(E\\) lines per set.

Figure 6.32: two-way set associative cache

### Set Selection in Set Associative Caches
同上 (Figure 6.33)

### Line Matching and Word Selection in Set Associative Caches
根据地址的 tag，比较 set 里面每一个 cache line，如果某一行的 valid bit = 1 且 tag 相同，则 cache hit.

Figure 6.34: an example.


### Line Replacement on Misses in Set Associative Caches
当发生 cache miss 时，需要从下一级的 memory 读出的 block，根据 addresss 选择一个 set，
但是一个 set 有多个 cache line，应该放在哪一个 cache line 中：

1. 如果有一行 cache line 是空的，则放入这一行
2. 随机选择一行，适用于 cache miss 代价不大的情况下
3. 选择 least frequently used (LFU) 的一行，需要额外的时间和硬件
4. 选择 least recently used (LRU) 的一行，需要额外的时间和硬件

## 6.4.4 Fully Associative Caches
\\(E=\frac{C}{B}\\) (\\(C\\): cache size, \\(B\\): block size)

只有一个 set，Figure 6.35.

### Set Selection in Fully Associative Caches
由于只有 1 个 set，所以不需要选择 set，address 里面有没有 set index。(Figure 6.36)

### Line Matching and Word Selection in Fully Associative Caches
同上，Figure 6.37.

Cache 电路需要并行地对多个 cache line 的 tag 与 address 中提取出来的 tag 进行比较，所以 it is difficult and expensive.

所以这种 cache 适用于 small caches，比如 TLB 和 cache page table entries.

## 6.4.5 Issues with Writes
Read:

1. 通过 address 在 cache 中查找相应的 block
2. Hit, 返回数据
3. Miss, 从 next lower level of memory hierarchy 中读取 block，然后存储在 cache 中的某一行中。

Write:
第一种情况，要写的数据在 cache 中，也称为 write hit，写入 cache 之后，何时去更新下一级的 memory?

* **Write-through**: 马上更新
* **Write-back**: 不马上更新，直到这个 cache block 需要被替换掉时才更新，需要有一个 *dirty bit* 来说明这个 cache block 是否被更新过

第二种情况，要写的数据不在 cache 中，也就是 write miss。怎么办？

* **Write-allocate**: 先把 data block 从下一级 memory 中读到 cache 中，再写；Write-back caches 一般用这种方法
* **No-write-allocate**: 绕过 cache，直接写到下一级 memory 中；Write-through caches 一般用这种方法

程序员些程序时，最好假设 write-allocate, write-back caches，因为：

1. 越低层的 memory 越有可能使用 write-back，这是因为数据传输时间长
2. 随着 logic density 的增加，write-back 所带来的相对复杂度变得越来越小
3. 和 read 操作对称，都 exploit locality

## 6.4.6 Anatomy (解剖) of a Real Cache Hierarchy
*i-cache*: a cache that holds instructions only

*d-cache*: a cache that holds program data only

*unified cache*: a cache that holds both instructions and data

现在的处理器往往采用独立的 i-cache 和 d-cache，原因：

1. 处理器可以同时读取 instruction 和 data
2. i-cache 是只读的，因此更简单。两种 cache 可以根据不同的 access pattern 进行优化，有不一样的 block sizes, associativities, capacities
3. 减少 data access 和 intruction access 的 conflict misses (但是可能是以增加 capacity miss 为代价的)

## 6.4.7 Performance Impact of Cache Parameters
衡量 cache 性能的指标：

* *Miss rate*: \\(# misses / # references\\)
* *Hit rate*: \\(1 - miss rate\\)
* *Hit time*: 从 cache 传输一个 word 到 CPU 的时间，包括 set selection, line identification, word selection
* *Miss penalty*: 由于 cache miss 导致的额外处理时间

### Impact of Cache Size
Cache Size 越大，hit rate 越高，但是往往 hit time 会越低

### Impact of Block Size
Block size 越大，对于 spatial locality 比较好的 program 来说 hit rate 越高。
但是对于 temporal locality 比 spatial locality 更好的 program 来说 hit rate 可能会变低。
因为 cache line 变少了 (假设 cache size 是一定的)。

并且 block size 越大，transfer time 越长，因此 miss penalty 越大。

### Impact of Associativity
Associativity 越高，conflict miss 发生的可能性越低，cost 越大，复杂性越高，因此 hit time 越长，miss penalty 越大（选择 victime line 的逻辑更复杂）。

L1 caches 往往采用低 associativity，低层的 memory 往往使用高 associative，因为 miss penalty 更严重。

### Impact of Write Strategy
Write-through 更易于实现。

使用 write-back 可导致更少的传输次数。越低层的 memory 越有可能采用 write-back，因为传输一次数据所需要的时间更长。

# 6.5 Writing Cache-Friendly Code
Basic approach:

1. *Make the common case go fast.*:
程序的大部分执行时间往往集中在少数的几个核心函数。
而核心函数的大部分执行时间往往花费在某几个 loop 上。所以集中优化核心函数的 inner loop 会非常有效。
2. *Minimize the number of cache misses in each inner loop.*
故名思议。

看懂 P635 和 P636 两段程序即可。

下面两段程序中，局部变量 `i`, `j`, `sum` 有良好的 temporal locality。
由于它们是局部变量，因此complier 往往会将它们存储在寄存器中。

第一段程序的 spatial locality 更好。

```c
int sumvec(int a[M][N])
{
    int i, j, sum = 0;
    for (i = 0; i < M; i++)
        for (j = 0; j < N; j++)
            sum += v[i][j];
    return sum;
}
```

```c
int sumvec(int a[M][N])
{
    int i, j, sum = 0;
    for (j = 0; j < N; j++)
        for (i = 0; i < M; i++)
            sum += v[i][j];
    return sum;
}
```

# 6.6 Putting It Together: The Impact of Caches: on Program Performance

## 6.6.1 The Memory Mountain
这一节探究了 temporal locality 和 spatial locality 和 read throughput 的关系。其中，read throughput 为单位时间一个程序读取的数据量，用 MB/s 来衡量。

使用的程序如 Figure 6.40 所示。
程序里有两个变量：`size` 和 `stride`。`size` 表示读取的总数据量，即 work set size；`stride` 表示循环的 step size。
`size` 体现的是 temporal locality: `size` 越小，temporal locality 越好。
`stride` 体现的是 spatial locality: `stride` 越小，spatial locality 越好。

不同的 `size` 和 `stride` 会导致不同的 read throughput。以 `size` 和 `stride` 为自变量，read throughput 为因变量的函数称为 *memory moutain*。

Figure 6.40 程序所产生的 memory moutain 如 Figure 6.41 所示。

单独研究 `size` 和 `stride` 对 read throughput 的影响，如 Figure 6.42 和 Figure 6.43 所示。

## 6.6.2 Rearranging Loops to Increase Spatial Locality
本节通过一个例子来说明如何提升 spatial locality。

所举的例子是矩阵相乘。
共有 6 种实现方式，如图 Figure 6.44 所示。
对 cache 的假设在 P644 页中间，每种实现方式的 cache miss 估计如 Figure 6.45 所示。

这 6 种实现方式的 performance 如 Figure 6.46 所示。最好情况下的性能比最差情况下的性能好 40 倍。

## 6.6.3 Exploiting Locality in Your Program
建议：

* Focus your attention on the inner loops, where the bulk of the computations and memory accesses occur.
* Try to maximize the spatial locality in your program by reading data objects sequentially, with stride 1, in the order they are stored in memory.
* Try to maximize the temporal locality in your programs by using a data object as often as possible once it has been read from memory
