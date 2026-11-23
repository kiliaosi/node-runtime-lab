# OS00-1：普通函数调用与系统调用

日期：2026-11-02。
状态：首个手写实验、正常/无效 fd 路径及 GDB 用户态边界检查已完成；OS00 仍在进行中。

平台：Linux x86-64；本轮实际内核为 5.14.0-687.10.1.el9_8.0.1.x86_64。strace 与 GDB 已存在。实验从仓库根目录运行。

## 本节要理解的边界

- 用户态和内核态是执行权限的区别；普通用户程序的代码通常在用户态由 CPU 执行。
- 自己编写的 add 函数可以在用户态完成计算，函数调用本身不要求进入内核。
- 调用 write 库封装后，通过系统调用进入内核请求写入；返回后继续执行用户代码。
- 进入内核态与切换到另一个线程是不同事情；系统调用处理可以在当前线程的上下文中进行。阻塞或抢占等原因也可能引发线程切换，但权限切换本身不要求更换线程。
- strace 展示系统调用事件，不是 C 函数调用日志；没有 add 记录不能据此推断它没有执行。

已检查 syscall 指令与寄存器，并参考上游 Linux v5.14 讲解入口/返回；没有单步调试当前 Rocky 内核。

## write 接口

```c
#include <unistd.h>
ssize_t write(int fd, const void *buf, size_t count);
```

fd 是当前进程中的文件描述符。STDOUT_FILENO 表示标准输出，通常为 1；它可以连接终端，也可以被重定向。buf 是待写字节的起始地址，count 是本次请求写出的字节数。

ssize_t 是有符号整数类型，可表示实际写出的字节数和错误值 -1。write 不依赖字符串结尾的 NUL；发送字符数组时用 sizeof(message)-1 排除结尾的 NUL。

返回 -1 表示错误并设置 errno，可立即调用 stdio.h 中的 perror("write") 打印该错误的文字说明。非负返回值也可能小于请求数；本节将短写视为本次任务未完成并返回非零，完整重试逻辑在后续 I/O 课实现。

接口与封装关系依据 [write(2)](https://man7.org/linux/man-pages/man2/write.2.html) 和 [syscalls(2)](https://man7.org/linux/man-pages/man2/syscalls.2.html)。

## 已完成的学生实现

1. 实现 int add(int a, int b)，返回两数之和。
2. 在 main 中调用 add(2, 3)，检查结果为 5；否则返回非零。
3. 定义字符数组，内容为 hello kernel 加一个换行。
4. 调用一次 write，向 STDOUT_FILENO 写出数组内容，不包含结尾的 NUL。
5. 用 ssize_t 保存返回值。失败时 perror 后返回非零；短写也返回非零；完整写出后返回 0。

正常路径本次只输出 hello kernel 一行，不额外打印求和结果，便于对应一次应用主动发起的 write。已用 GDB 观察 add 返回 5，结果见下方验证记录。

## 构建与观察

```bash
source scripts/activate-lab.sh &&
cmake -S labs/00-foundations/os-runtime/00-syscalls -B build/f03-os00 -DCMAKE_BUILD_TYPE=Debug &&
cmake --build build/f03-os00 --target f03-os00-syscalls &&
strace -e trace=write,exit_group ./build/f03-os00/f03-os00-syscalls
```

VS Code Remote SSH：F1 → Tasks: Run Task → F03 OS00: build and trace syscalls。
构建使用 C11、-O0、-g 与严格警告。构建失败不会运行旧程序。

正常路径实际程序输出：

```text
hello kernel
```

正常路径实际系统调用记录的核心行：

```text
write(1, "hello kernel\n", 13) = 13
```

13 包含换行，不包含 NUL；终端上的程序输出与 strace 记录可能交错。
正常路径完整写入 13 字节，进程退出 0。

## 验证记录与续学

- 无效 fd 变体已实测：write(-1, ...) 返回 EBADF；perror 向 fd 2 输出错误；main 返回 -1，外部退出状态 255。源码已恢复 STDOUT_FILENO。
- 助手 GDB 运行验证 add 返回 5，以及 libc 的实际 syscall 指令前 rax=1、rdi=1、rdx=13，返回后 rax=13；用户态 GDB 未进入内核实现单步。
- 日志位于 build/f03-os00：syscalls-check.trace、syscalls-invalid-fd.trace、gdb-boundaries.log、gdb-syscall-instruction.log。首个 stdout 程序只检查了短写分支，未对它主动制造真实短写；后续 file_write 程序已实际验证 11→5 字节短写。
- 学生已正确复述库调用与系统调用的往返，并报告亲手观察到重定向前后 fd 均为 1、目标从终端变为 stdout.txt。
- 已讲解 strace 的记录范围和退出库函数/系统调用区别。

[自己管理文件描述符：open → write → close](FILE_WRITE.md)已通过本轮验收；最终正常运行与模拟 close 失败，以及此前真实短写等证据见该文档。

后续已验证的实验：

- [文件描述符的复用](FD_REUSE.md)（fd_reuse.c）：正常运行和两个 open 失败分支已验证。
- 文件偏移对照（fd_offset.c）：严格构建通过，dup 共享偏移的文件为 ABCX，独立 open 的文件为 XBC。

进程与调度的续学记录见 [F03 笔记](../../../../notes/F03-process-scheduling.md)。

本日学习笔记：[OS00（一）：系统调用边界与文件描述符生命周期](../../../../notes/OS00-syscalls-part1.md)。
