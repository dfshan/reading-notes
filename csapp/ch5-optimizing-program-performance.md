# 5.1 Capabilities and Limitations of Optimizing Compilers
`GCC` 可以通过 `-O` 选项来确定 optimizing level.

* `-Og`: A basic set of optimizations
* `-O1`, `-O2`, `-O3`: More extensive optimizations

Compiler 只能对 program 进行 *safe* optimization：
优化后的 program 和优化前的 program 有相同的 behavior.

容易导致 unsafe optimizing 的两个原因：

  * memory aliasing: 两个指针指向同一个 memory location
  * 函数内部调用全局变量

例子：Memory aliasing (两个指针指向同一个 memory location) 导致的 unsafe optimizing:

```c
void twiddle1(long *xp, long *yp) {
    *xp += *yp;
    *xp += *yp;
}
void twiddle2(long *xp, long *yp){
   *xp += 2 * *yp;
}
```

这两个函数看似结果一样，但是如果 `xp` 和 `yp` 指向的是同一个地方的话，
会产生不一样的结果：前者 `*xp` 会增加 4 倍，而后者仅仅增加 3 倍。
z
另外一个例子：
```c
x = 1000; y = 3000;
*q = y;
*p = x;
t1 = *q;
```
上述程序中，`t1` 的值不一定是 1000.


2. 另外一个例子：函数内部更改全局变量
```c
long f();
long func1() {
    return f() + f() + f() + f();
}
long func2() {
    return 4*f();
}
```
如果 `f()` 函数更改了全局变量，那么 `func1()` 和 `func2()` 的结果可能不一样:
```c
long counter = 0;
long f() {
    return counter++;
}
```

**Inline substitution**:
在调用函数时，把调用函数的那行代码替换为整个函数的代码。
这样就减少了一次函数调用，便于编译器做优化。

例如，在上面的 `func1()` 函数可以变为：
```c
long funclin() {
  long t = count++;
  t += counter++;
  t += counter++;
  t += counter++;
  return t;
}
```
这样一来，编译器就可以把上面的代码优化为：
```c
long funclopt() {
  long t= 4 * counter * 6;
  counter += 4;
  return t;
}
```
`GCC` 可以使用 `-finline` 或 `-O1` and higher 选项来使用这个功能。
但是，`GCC` 仅仅对定义在同一个文件中的函数进行 inlining.

# 5.2 Expressing Program Performance
使用 cycles per element 来衡量一个 program 的性能。

注意：不是 cycles per loop，因为一个 program
可能会使用 loop unrolling 技术（比如每次 loop 对两个 element 进行处理）。

# 5.3 Program Example
本章所使用的示例程序。如 Figure 5.4 所示。
该程序是对 Figrure 5.3 所示的一个 vector 数据结构进行操作。

Figure 5.5 是一个测试性能的程序，
该程序对 vector 里的所有数据进行合并（比如相加，相乘）。
具体的合并操作通过 `OP` 宏来定义。

P508 表格是一些初步测试结果。

# 5.4 Eliminating Loop Inefficiencies
*Code motion* 技术：
把一段被执行很多次（比如在一个循环中）但是结果却不变的代码识别出来，
然后把这段代码移到之前的位置（比如循环之前），
这样这段代码就不用执行多次了。

例子：在一个循环中调用一个函数，但是函数的返回值不变。
可以把函数返回的结果在循环之前算出来，然后在循环内部直接使用这个结果。

Figure 5.7: 每次循环都要调用 `strlen` 函数，
而 `strlen` 函数需要 $O(n)$ 的执行时间，
导致该算法有 $O(n^2)$ 的执行时间。
但是，`strlen` 函数每次执行的返回值均不变，
而编译器在编译代码时，要识别这个特征并不容易。
所以，把 `strlen` 函数放在循环之前执行会大大提高代码运行时间 (Figrure 5.8)。

# 5.5 Reducing Procedure Calls
由于编译器是非常保守的，它得保证代码的正确性。
所以，在一段代码中，如果有函数调用， 有的编译器可能不会对代码进行优化。
（为什么？对存在函数调用的优化可能产生的问题，例如 5.1 P500 例子）。

另外一方面，函数调用本身也会产生开销（比如参数压栈）
所以，最好能减少函数调用的次数。

例子：Figure 5.6 的　`combine2` 函数可以转换为 Figure 5.9 的　`combine3` 函数．
但是　`combine3` 的性能并没有比　`combine2` 好多少 (为什么？5.11.2)。
所以我觉得这不是一个好例子。

# 5.6 Eliminating Unneeded Memory References
直接看例子。
Figure 5.9 的 `combine2` 函数可以转换为 Figure 5.10 的 `combine3` 函数。
好处是减少了读写内存的次数。

为什么编译器不能做这种优化？因为 memory aliasing。
比如 `dest` 如果指向的是 vector 内部的某一个数据，
那么 `combine3` 和 `combine4` 可能会产生不同的结果。

例子：P515, 下面两行代码的执行结果是不一样的：

```c
combine3(v, get_vec_start(v) + 2);
combine4(v, get_vec_start(v) + 2);
```

# 5.7 Understanding Modern Processors
Processor 的一个重要特性：
instruction-level parallelism：
100 多条指令同时执行，但是执行结果和每条指令顺序执行的结果一样。

## 5.7.1 Overall Operation
非常详细地解释了现代处理器的工作过程。有时间可以仔细看看。
重点需要掌握以下概念：

Out-of-order processor 有两个模块:

1. Instruction control unit: 从 cache 中读取指令， 把指令解析出来，做 prediction
2. execution unit: 执行操作

Speculative execution: 根据预测出一个分支， 提前执行分支下的操作。
在不知道分支预测是否正确之前，对内存和寄存器的修改不会被真正执行。
如果分支预测正确， 所有对寄存器的修改会被写入到寄存器中；
如果分支预测不正确，所有计算出来的结果都会被忽略掉，重置状态，重新执行。

流水线：指令可以拆分成多个阶段，流水线执行

并行：一个处理器可能有多个处理模块，这样多个操作（比如加法）可以并行执行

## 5.7.2 Functional Unit Performance
* **Latency bound**: 顺序执行一个操作所需要的最少时间
* **Throughput bound**: 考虑到流水线，多个功能模块等因素下，执行一个操作所需要的最少时间

## 5.7.3 An Abstract Model of Proessor Operation
需要能看懂 Figure 5.14 和 Figure 5.15 两个图，后面可能会用到。

### From Machine-Level Code to Data-Flow Graphs
Critical path: 执行时间最长的一条 path.

### Other Performance Factors

# 5.8 Loop Unrolling
Loop unrolling: reducing the number of interations for a loop
by increasing the number of elements on each iteration.

为什么 loop unrolling 能提高性能：

1. 减少不必要的开销，比如 loop indexing (i++)，和 conditional branching (i &lt; n)
2. 我们可以进一步优化代码，以便能减少 critial path 的 operation 次数

在 Figure 5.16 所示的 `combine5` 是一个 roop unrolling 的例子.

为什么？把这个例子用 data-flow 画出来，如 Figure 5.20 所示，
虽然循环次数减少了一半，但是每次循环需要做两次乘法操作，
所以总的时间没有变化。

后面这一节会对这个例子做进一步优化。

Aside: `GCC` will perform some forms of loop unrolling
when invoked with optimization level 3.

# 5.9 Enhancing Parallelism
之前程序的问题：后一个 computation 必须要在前一个 computation 完成之后才能进行，
以至于不能进行 pipeline 以及利用多个 computation units.

## 5.9.1 Multiple Accumulators
`combine5` 可以优化为 `combine6`，如 Figure 5.21 所示。

为什么优化后的程序性能能提高：因为可以进行 pipeline，以及指令级并行。
data-flow 图如 Figure 5.24 所示。

优化之后的程序结果和优化之前的结果一样的前提条件是 `OP` 操作满足结合律。
但是 floating-point multiplication and addition 不满足结合律、
(due to rounding or overflowing)，
因此大部分的编译器都不会对 floating-point code 进行类似的优化。

## 5.9.2 Reassociation Transformation
`combine5` 可以优化为 `combine7`，如 Figure 5.26 所示。

优化的地方在于

```c
acc = (acc OP data[i]) OP data[i+1]
```

转换为下面的代码

```c
acc = acc OP (data[i] OP data[i+1])
```
这样一来，`(data[i] OP data[i+1])` 这个计算不依赖与上一个循环的执行结果，
所以可以在上一个循环结束之前就执行。

Data-flow 图如 Figure 5.28, 5.29 所示。

# 5.10 Summary of Results for Optimizing Combining Code
一个总结，对比了各个程序的性能之后，得出结论：
modern processors have considerable amounts of computing power,
but we may need to coax this power out of them by
*writing our programs in very stylized ways*.

# 5.11 Some Limiting Factors
## 5.11.1 Register Spilling
当一个程序的并发程序 P 超过了寄存器的数目时，compiler 会把一些中间结果存到 memory 中，
导致性能变差。

比如 (P548) 在 `combine6` 程序中，$20 \times 20$ unrolling 的性能比 $10 \times 10$ 的 性能要差。
因为 modern x86-64 processors 通常只有 16 个寄存器。

## 5.11.2 Branch Prediction and Misprediction Penalties
如果分支预测不正确，instruction pipeline 需要被 refilled，所以会产生 misprediction penalty.

Conditional statements (a &lt; b ? a : b) 不会产生 misprediction penalty:
因为它可以用 conditional move 指令来实现，而 conditional move 指令可以作为 pipeline 的一部分。

### Do Not Be Overly Concerned about Predictable Branches
如题：大部分的分支预测模块都能看出分支指令的 regular patterns and long-term trends。
比如，循环语句中的 loop closing 判断语句一般预测为真，只有最后一次 loop 才会产生 misprediction.

### Write Code Suitable for Implementation with Conditional Moves
分支预测只有对 regular pattern 才有效果，
而程序中很多 test 都是 unpredictable 的。
所以最好把一些语句转换为 conditional move 语句。

比如，可以通过 conditional operations 来计算一些值，
然后根据这些值来更新 program state.

例子： P552 页的两个程序。

# 5.12 Understanding Memory Performance
Modern processors 有专门的 load and store units，
这些 units 有内部缓存来 hold outstanding requests.
比如，书中的 reference machine 有 2 个 load units，1 个 store units。
每个 load unit 可以最多 hold 72 个正在进行的 read requests，
每个 store unit 可以最多 hold 42 个正在进行的 write requests.

## 5.12.1 Load Performance
感觉没有什么 takeaway，就是告诉我们 load 操作也需要时间。

## 5.12.2 Store Performance
Write-read dependency: 从之前 write 过的 memory location 读取数据，
所以 memory read 的结果取决于之前 memory write 的结果。

处理器 load 操作的执行过程 (Figure 5.34)：
首先，在 store unit 中有一个 store buffer，
存储着所有尚未执行完的 store 操作的 addresses 和 data.
Load 操作执行时，会在 store buffer 中检查是否有 address 和 load 的 address 匹配，
如果存在，则直接从 store buffer 中读取 data 作为 load 操作的结果。

例子：Figure 5.33.
Store 操作分为两步：计算出　store 操作的地址并存储到　store buffer 中，
把 store 操作的数据存储在　store buffer 中。
当存在 write-read dependency 时，load 操作要等待 store 操作的后一步完成，
才能继续执行。
所以，就会产生 Figure 5.36 和 Figure 5.37 所示的 data flow。

可以通过 Practice Problem 5.11 & 5.12 来理解怎么来消除 write-read dependency.
# 5.13 Life in the Real World: Performance Improvement Techiques
优化程序的策略：

* High-level design: 使用合适的算法和数据结构
* Basic coding principles
  - Eliminate excessive function calls. 如果可以的话，把 computation 移到循环以外执行。
  - Eliminate unnecessary memory references. 使用 temporary 变量来存储中间结果，把最终结果计算出来之后才才把结果存到 array 或全局变量中。
* Low-level optimizations
  - Unrolling loops 以减少 overhead，且供进一步优化
  - Increase instruction-level parallelism
  - Rewrite conditional operations in a functional style，以便使用 conditional data transfer 语句

# 5.14 Identifying and Eliminating Performance Bottlenecks
通过 *code profilers* (analysis tools that collect performance data about a program as it executes) 来分析程序的各个部分的执行时间，因而找到需要改进的位置。

## 5.14.1 Program Profiling
Unix 系统中提供了一个名为 `GPROF` 的 profiling program.
它产生两类数据：
  * How much CPU time was spent for each of the functions in the program
  * How many times each function gets called, categorized by which function performs the call

使用 `GPROF`:

1. 编译。需要增加 `-pg` 选项，不能使用 inline substitution 来优化代码
```shell
linux> gcc -Og -pg prog.c -o prog
```
2. 执行程序，会生成 `gmon.out` 文件
```shell
linux> ./prog file.txt
```
3. 分析文件 `gmon.out`
```shell
gprof prog
```

三个需要注意的地方：

* 函数执行时间不是非常精确。精度大概在 1.0-10.0 ms。
为什么？函数执行时间是这么算的，
每隔一定的时间 ($\sigma$) 去中断一次 program，
然后找到 program 正在执行的 function，
给该 function 的执行时间增加 $\sigma$。
* Calling information (调用和被调用次数) 是非常 reliable 的
* 默认情况下不会显示库函数的 timings

**其它复杂的 profilers**:
`VTUNE` from Intel, `VALGRIND` 等等。
这些程序可以以 basic block 为单位来分析程序的执行时间
(一个 basic block 是一段 instruction，且中间不会进行跳转)。

## 5.14.2 Using a Profiler to Guide Optimization
可以看看，如何一步一步地对程序进行优化。
优化后的程序比优化前的程序快近 $1,000\times$。
