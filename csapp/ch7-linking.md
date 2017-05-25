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
来自于其它 module 的全局变量和函数，
以及只能由本 module 访问的 static 全局变量和函数。

还有一类 symbol 也在 symbol table 中，即函数中定义的局部 static 变量。

Symbol table entry 的格式：Figure 7.4.

可以使用 GNU `READELF` 程序来查看 object files 的内容。

# 7.6 Symbol Resolution
# 7.7 Relocation
# 7.8 Executable Object Files
# 7.9 Loading Executable Object Files
# 7.10 Dynamic Linking with Shared Libraries
# 7.11 Loading and Linking Shared Libraries from Applications
# 7.12 Position-Independent Code (PIC)
# 7.13 Library Interpositioning
# 7.14 Tools for Manipulating Object Files
