The linker combines different part of our program into a single file
that can be loaded into memory and execute by the processor.
# 7.1 Compiler Drivers
什么是 compiler driver? 比如 `gcc` 就是。

通过例子来理解编译链接过程：
Figure 7.1 有两个程序，`main.c` 调用 `sum` 函数，`sum` 函数在 `sum.c` 文件中。

编译过程：首先分别对 `main.c` 和 `sum.c` 进行 preprocessing, compilation, assembly,
最后把两个 *relocatable object file* (.o 文件) 进行 linking (`ld` 命令)。

# 7.2 Static Linking
Static linker (如 `ld`) 以多个 relocatable object 文件为输入，输出一个 fully linked executable object file.

Linker 必须干两件事情:

1. Symbol resolution:
把 function, global variable, static variable 和 symbol reference 关联起来。
(编译原理里的建立符号表？)

2. Relocation: 给 code 和 data section 分配内存地址，
然后使上一步的 symbol reference 指向这些地址。

# 7.3 Object Files
有三种 object file:

* Relocatable object file: 包含二进制的 code and data，可以和其它的 relocatable object file 合并成一个 executable object file.
* Executable object file: 可以直接被拷贝到内存执行的文件
* Shared object file: 一种特殊的 relocatable object file, which can be loaded into memory and linked dynamically, at either load time or run time.


# 7.4 Relocatable Object Files
Figure 7.3 shows the format of a typical ELF relocatable object files.
PS: 不同的系统使用不同的 object file 格式。x86-64 Linux 使用的是 Executable and Linkable Format (ELF).

# 7.5 Symbols and Symbol Tables
包含全局变量，不包含局部变量，因为局部变量在 stack 中。

包括在此 module 中定义的 nonstatic 全局变量和函数 (能被其它 module 访问),
来自于其它 module 的全局变量和函数 (extern)，
以及只能由本 module 访问的 static 全局变量和函数。

还有一类 symbol 也在 symbol table 中，即函数中定义的局部 static 变量。

Symbol table entry 的格式：Figure 7.4.

可以使用 GNU `READELF` 程序来查看 object files 的内容。

# 7.6 Symbol Resolution
Symbol resolution 的目标是 associate each symbol reference in the code with exactlyh one symbol definition.

从一个常见的问题开始说起。
为什么一些 C 程序在编译没有问题，但是在 linking 的时候会出现 "undefined reference to ..." 错误？例子 P679

当编译器遇到一个没有在当前 module 里定义（注意，此处不是声明）的 global symbol (变量或函数)，
编译器就会假定该 symbol 在其它 module 中定义来，
并且生成一个 linker symbol table entry，
交给 linker 来处理。
如果 linker 找不到定义，则会发生上述错误。

前面讲的是没有定义 symbol，还有一种可能是有多个地方对 symbol 进行了定义。
linker 要么 flag an error，要么选择一个定义，忽略其它定义。
具体处理情况如下。

## 7.6.1 How Linkers Resolve Duplicate Symbol Names
每一个 realocatable object modules 都定义了一些 symbols。
本节讨论当不同 module 定义相同名字的 symbol 时如何处理。

两个概念:
Functions and initialized global variables get *strong* symbols.
Uninitialized global variables get *weak* symbols.

三条 rule:

Rule 1. 不允许出现多个相同名字的 strong symbols

Rule 2. 如果出现 1 个 strong symbol 和多个 weak symbols，选择 strong symbol

Rule 3. 如果出现多个 weak symbols，选择其中一个

例子：P680 - P683

注意 P682 最后一段程序出现的 run-time bugs.

`GCC` 可以使用 `-fno-common` 来 trigger an error if it encounters multiply-defined global symbols.

## 7.6.2 Linking with Static Libraries
我们可以把一堆 related object modules 打包装入一个 *static library* 中。
在 linking 的时候，linker 只会使用 library 里面用到的 module。
例如我们如果只使用了 `printf` 函数，那么只有 `printf.o` 才会被拿出来 linking。
At link time, the linker will only copy the object modules that are referenced by the program.

在 Linux 中，static libraries 存储在 `.a` 结尾的文件中，knonw as *archive*。

例子：Figure 7.6 & Figure 7.7.

Create a static library (使用 `AR` tool):
```shell
linux> gcc -c addvec.c multvec.c
linux> ar rcs libvector.a addvec.o multvec.o
```

Build executable:
```shell
linux> gcc -c main2.c
linux> gcc -static -o prog2c main2.o -L. -lvector
```
上面 `gcc` 命令的参数中，`-static` 表示需要 build a fully linked executable object file
that can be loaded into memory and run without further linking at load time.
`-lvector` 参数相当于 `libvector.a`，`-L.` 参数告诉 linker 寻找 static library 的路径。

## 7.6.3 How Linkers Use Static Libraries to Resolve References
在 symbol resolution phase, linker 会从左到右扫描 relocatable object files 和 archives.
比如 `gcc foo.c libx.a liby.a libz.a`，
linker 会先扫描 `foo.o` (假设已经生成该文件)，
然后扫描 `libx.a`, 最后扫描 `libz.a`。

在扫描过程中，有三个集合：

* 集合 E: 包含所有需要合并到 executable file 里的 relocatable object files
* 集合 U: 包含所有 unresolved symbols (symbols referred to but not yet defined)
* 集合 D: 在之前的输入文件中定义的 symbols

扫描过程中的处理如下

* 对于每个输入文件 f，如果 f 是一个 object file，则 linker 把 f 加入到集合 E，并且更新集合 U 和 D
* 如果 f 是一个 archive，linker 尝试在 archive 中找到集合 U 中的 unresolved symbols.
如果集合 U 中有一个 symbol 在 archive 文件中的 module m 定义，那么把 module m 加入到集合 E 中，并且更行集合 U 和 D.
* 扫描结束时，如果集合 U 非空，则 linker 会 prints an error and terminates

根据这个过程，可以解释一些以前不理解的 link time errors。比如下面的错误

```shell
linux> gcc -static ./libvector.a main2.c
/tmp/cc9XH6Rp.o: In function 'main':
/tmp/cc9XH6Rp.o (.text+0x18): undefined reference to 'addvec'
```

当 linker 在处理 `libvector.a`	时，集合 U 是空的，所以 `libvector.a` 里面没有 object file 会加入到集合 E，
因而，`main2.c` 文件里面的 symbol `addvec` 就会找不到定义了。

<font color=red>**Rule**: 在编译时，把 static library 放到最后 (relocatable object file 后面)。</font>

编译命令中，static library 也可以重复。
比如，`foo.c` calls a function in `libx.a`,
`libx.a` calls a function in `liby.a`,
`liby.a` calls a function in `libx.a`，编译命令为：

```shell
linux> gcc foo.c libx.a liby.a libx.a
```
# 7.7 Relocation
Two steps:

1. Relocating sections and symbol definitions:
	* 合并来自不同的 modules 的相同 section. 比如，把不同 modules 中的 .data section 合并成一个 .data section.
	* Assign run-time memory addresses to the new aggregated sections, to each section defined by the input modules, and to each symbol defined by the input modules.
2. Relocating symbol referencesd within sections:
把 code and data sections 中的 symbol reference 改为 run-time addresses.

## 7.7.1 Relocation Entries
Relocating symbol 阶段通过 relocation entries 这个数据结构来进行。
当 assembler 遇到一个无法确定 location 的 symbol reference 的时候，
它会生成一个 relocation entry，以便 linker 去把 reference 更改为 run-time address.
Relocation entry 的格式如 Figure 7.9 所示。

Relocation entry 有两种比较常用的类型（不仅仅只有这两种）：

* `R_X86_64_PC32`: 相对地址，or 32-bit PC-relative address. 也就是相对于 program counter 的偏移量
* `R_X86_64_32`: 32-bit absolution address

## 7.7.2 Relocating Symbol References
Relocation 过程的伪代码如 Figure 7.10 所示。

通过一个例子来说明这个过程。
Figure 7.11 是 Figure 7.1 程序的汇编代码。

### Relocating PC-Relative References
Relocate `sum` 的具体过程。

### Relocating Absolute References
Relocate `array` 的具体过程。

# 7.8 Executable Object Files
所有的 object files 被合并成 1 个 executable object 文件。
Figure 7.13: ELF executable object 文件格式。

# 7.9 Loading Executable Object Files
Figure 7.15: Linux x86-64 run-time memory image.

运行 executable object file 的 program 叫 `loader`，
对于 Linux，可以通过 `execve` 函数来调用 loader.

P699 Aside, how do loaders really work:
当 shell 运行一个 program 时，它首先会 `fork` 一个子进程，然后在子进程中通过 `execve` 系统调用运行 loader.
Loader 会删掉子进程现有的 virtual memory segments，然后创建新的 code, data, heap 和 stack segments.
The new stack and heap segments are initialized to zero.
The new code and data segments are initialized to the contents of the executable file
by *mapping pages* in the virual address space to page-size chunks of the executable file.
然后，loader jmps to the program's entry point。
一个 program 的 entry point 是 `_start` 函数，该函数在 symstem object file `crt1.o` 中定义，
`_start` 函数会调用 *system startup function* --- `__libc_start_main`，
该函数在 `libc.so` 中定义，
它会初始化 execution environment，并调用 user-level `main` function.

注意，copying of data from dist to memory during loading 是 on demand 的。

# 7.10 Dynamic Linking with Shared Libraries
可以结合 P833 Section 9.8.1 来看。

Static libraries 有两个缺点：

1. 当我们更新一个 static library，
所有使用该 library 的 program 都需要重新 linking，
才能使用更新后的 static library.
2. 如果有多个 program 都使用了同一个 static library 的一个 module (比如 printf.o)，
那么这个 module 会存在于所有 program 的内存当中，
这样会造成内存浪费。

因此有了 shared library 的概念。
Shared library 在 run time 或者 load time 才会被 load 到 memory 里 并进行 linking。
这个过程叫 *dynamic linking*。
在 Linux 中，shared library 又叫 *shared objects*，是以 `.so` 结尾的文件，
例如 C standard library `libc.so`；
在 Windows 中，shared library 叫 dynamic link libraries (DLL).

创建 shared library:
```shell
linux> gcc -shared -fpic -o libvector.so addvec.c multvec.c
```

使用 shared library 编译：
```shell
linux> gcc -o prog21 main2.c ./libvector.so
```

执行完上一步之后，none of code and data sections from `libvector.so` are actually copyied into executable `prog21`.
Linker 只是把一些 relocation and symbol table information 拷贝进去了，用来 resolve references to code and data in `libvector.so`.

Dynamic linker 发现 executable object file 有 dynamic library 之后，它执行以下步骤：

* Relocating the text and data of `libc.so` into some memory segment
* Relocating the text sand data of `libvector.so` into another memory segment
* Relocating any reference in `prog21` to symbols defined by `libc.so` and `libvector.so`

# 7.11 Loading and Linking Shared Libraries from Applications
应用程序执行期间也可以动态地 load and link shared libraries.

Linux interfaces:

* `dlopen`: loads and links a shared library
* `dlclose`: unloads the shared library if no other shared libraries are still using it
* `dlsym`: get the address of a symbol
* `dlerror`: returns a string describing the most recent error

例子 Figure 7.17

# 7.12 Position-Independent Code (PIC)
Shared library 中 symbol 的 run-time memory address 在 linking 的时候是不确定的，
同一模块内部的 address 可以使用 PC-relative addressing 来实现
(shared library 中的 code segment 和 data segment 是大小是固定的，所以相对偏移地址也是可以确定的)，
但是 references to external procedures and global variables 需要使用 PIC.
### PIC Data References
Figure 7.18: data segment 的起始部分有一张 global offset table (GOT)，
这个 module 里面 reference 的每一个 procedure or global variable
都在这张表里面的有一项对应的表项。
At load time, dynamic linker relocates each GOT entry so that it contains the absolute address of the object.
该图中，PIC 所使用的地址是相对于当前指令的相对地址，该地址指向 GOT 表中的一项，
而表项的内容就是 `addcnt` 的地址。

### PIC Function Calls
GNU compilation 使用了一种叫 *lazy binding* 的技术。
基本思想是当第一次调用 shared library 里面的函数时，
程序会通过 dynamic linker 来确定这个函数的地址，然后再调用这个函数。
后续调用同样的函数就不需要 dynamic linker 了，因为这个函数的地址已经被存在一张表格当中了。

具体的实现方法见 P706.

# 7.13 Library Interpositioning
Library interpositioning 是一种可以劫夺 shared library 里函数的一种方法，把函数替换成自己的 code.
Interpositioning 可以发生在 compile time, link time, **or** runtime.

例子：Figure 7.20(a) 中的程序调用了 `malloc` 和 `free` 函数，
接下来通过 interpositioning 来跟踪 malloc and free 的调用。

## 7.13.1 Compile-Time Interpositioning
如 Figure 7.20 所示。
在 `malloc.h` 和 `malloc.c`	中定义和标准库函数中的 `mallc` 和 `free` 函数一样的函数。
编译时使用下面的命令：
```shell
linux> gcc -DCOMPILETIME -c mymalloc.c
linux> gcc -I. -o intc int.c mymallo.c
```
上面的命令中，`-DCOMPILETIME` 定义了一个 macro `COMPILETIME`。为什么要这么做，看 Figure 7.20(c) 第一行。

`-I.` 参数告诉 C preprocessor to look for `malloc.h` in the current directory before looking in the usual system directories.

## 7.13.2 Link-Time Interpositioning
Linux static linker 可以通过 `--wrap f` 参数来实现 link-time interpositioning.
这个参数的含义是把 symbol `f` 解析为 `__wrap_f`，而把 `__real_f` 解析为 `f`。

例子：Figure 7.21。
编译时使用如下命令：
```shell
linux> gcc -DLINKTIME -c mymalloc.c
linux> gcc -c int.c
```
Linking 时使用如下命令：
```shell
linux> gcc -Wl,--wrap,malloc -Wl,--wrap,free -o intl int.o mymalloc.o
```
上面的命令中 `-Wl,` 表示后面跟着的是要传给 linker 的 options。
所有的逗号会被空格取代。
因此，`-Wl,--wrap,malloc` 相当于给 linker 传了参数 `--wrap malloc`。

## 7.13.3 Run-time Interpositioning
通过一个环境变量 `LD_PRELOAD` 来实现，这个环境变量是一些裂 shared library 的路径，
在 load and execute a program 时，dynamic linker 会优先搜索 `LD_PRELOAD` 内的 shared library。

例子，Figure 7.22.
编译时使用如下命令
```shell
linux> gcc -DRUNTIME -shared -fpic -o mymalloc.so mymalloc.c -ldl
linux> gcc -o intr int.c
```
Note: `-ldl` 指示连接器连接一个库。这个库里包含了 dlopen, dlsym 等等的函数。也就是说，是支持在运行时加载使用动态连接库的函数库。

运行程序:
```shell
LD_PRELOAD="./mymalloc.so" ./intr
```

# 7.14 Tools for Manipulating Object Files
一些工具：

* `LDD`: 列出一个 executable 用到的所有 shared libraries
* `AR`: 创建 static libraries，查看和修改 members
* `STRING`: 列出所有 object file 中的 printable strings
* `STRIP`: 从 object file 中删除 symbol table information
* `NM`: 列出所有在 symbol table 中定义的 symbols
* `SIZE`: 列出 object file 中的 sections 的 names and sizes
* `READELF`: 显示 object file 的完整结构
* `OBJDUMP`: 可以显示 object file 的所有信息
