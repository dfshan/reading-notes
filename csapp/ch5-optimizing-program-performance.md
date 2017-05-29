# 5.1 Capabilities and Limitations of Optimizing Compilers
`GCC` 可以通过 `-O` 选项来确定 optimizing level.

* `-Og`: A basic set of optimizations
* `-O1`, `-O2`, `-O3`: More extensive optimizations

Compiler 只能对 program 进行 *safe* optimization：
优化后的 program 和优化前的 program 有相同的 behavior.

Memory aliasing (两个指针指向同一个 memory location) 导致的 unsafe optimizing:

例子：

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


另外一个例子：函数内部更改全局变量
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
`GCC` 可以使用 `-finline` 或 `-O1` and higher 选项了使用这个优化。
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
所以，在一段代码中，如果有函数调用，
有的编译器 optimization 技术可能没办法进行。
（为什么？对存在函数调用的优化可能产生的问题，
例如 5.1 P500 例子）。

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
100 多条指令同时执行，但是执行结果和每条指令单独执行的结果一样。

## 5.7.1 Overall Operation
非常详细地解释了现代处理器的工作过程。有时间可以仔细看看。
重点需要掌握以下概念：

Out-of-order 有两个模块:

1. Instruction control unit: 从 cache 中读取指令，
把指令解析出来，作 prediction
2. execution unit: 执行操作

Speculative execution: 根据预测出一个分支，
提前执行分支下的操作，如果分支预测正确，
所有对寄存器的修改会被写入到寄存器中；
如果分支预测不正确，所有计算出来的结果都会被忽略掉，重置状态，重新执行。

流水线：指令可以拆分成多个阶段，流水线执行

并行：一个处理器可能有多个处理模块，这样多个操作（比如加法）可以并行执行

## 5.7.2 Functional Unit Performance
## 5.7.3 An Abstract Model of Proessor Operation
需要能看懂 Figure 5.14 和 Figure 5.15 两个图，后面可能会用到。

### From Machine-Level Code to Data-Flow Graphs
Critical path: 执行时间最长的一条 path.

### Other Performance Factors

# 5.8 Loop Unrolling
Loop unrolling: reducing the number of interations for a loop
by increasing the number of elements on each iteration.

为什么 loop unrolling 能提高性能：

1. 减少不必要的开销，比如 loop indexing (i++)，和 conditional branching (i < n)
2. 我们可以进一步优化代码，以便能减少 critial path 的 operation 次数

在 Figure 5.16 所示的 `combine5` 是一个 roop unrolling 的例子.

为什么？把这个例子用 data-flow 画出来，如 Figure 5.20 所示，
虽然循环次数减少了一半，但是每次循环需要做两次乘法操作，
所以总的时间没有变化。

后面这一节会对这个例子做进一步优化。o

Aside: `GCC` will perform some forms of loop unrolling
when invoked with optimization level 3.

# 5.9 Enhancing Parallelism
## 5.9.1 Multiple Accumulators
## 5.9.2 Reassociation Transformation
