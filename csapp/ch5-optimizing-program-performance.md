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
# 5.5 Reducing Procedure Calls
# 5.6 Eliminating Unneeded Memory References
# 5.7 Understanding Modern Processors
## 5.7.1 Overall Operation
## 5.7.2 Functional Unit Performance
## 5.7.3 An Abstract Model of Proessor Operation
### From Machine-Level Code to Data-Flow Graphs
### Other Performance Factors
# 5.8 Loop Unrolling
# 5.9 Enhancing Parallelism
## 5.9.1 Multiple Accumulators
## 5.9.2 Reassociation Transformation
