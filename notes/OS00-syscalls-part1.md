# OS00（一）：系统调用边界与文件描述符生命周期

日期：2026-11-02。状态：本轮两个 C 实验已通过；OS00 整体进行中；fd 复用练习已布置、尚未实现。

代码与任务：[实验入口](../labs/00-foundations/os-runtime/00-syscalls/README.md)、[文件写入](../labs/00-foundations/os-runtime/00-syscalls/FILE_WRITE.md)、[下次的 fd 复用](../labs/00-foundations/os-runtime/00-syscalls/FD_REUSE.md)。

环境：Linux x86-64，Rocky 内核 5.14.0-687.10.1.el9_8.0.1.x86_64，GCC Toolset 14。内核入口源码以另行阅读的上游 Linux v5.14 为参考，不能等同于当前发行版的完整补丁实现。

## 1. 一次 write 的路径

```text
main 的普通函数调用
  → 用户态 libc write
  → 准备调用号和参数，执行 syscall
  → CPU 转到内核预设入口
  → 内核入口保存现场、切换内核栈
  → 按系统调用号分派到 write
  → 查 fd，校验参数，调用目标对应的写入实现
  → 内核准备返回值并返回用户态
  → libc 检查错误，必要时设置 errno
  → 返回 main，继续执行
```

当前线程可以在内核态完成这次服务。权限切换不必切换到另一个线程；阻塞、抢占等情况可能引起调度。用户态 GDB 观察的是边界与用户代码，本轮未单步调试运行中的内核。

CPU 的 syscall 指令使用内核预先配置的入口地址。硬件保存返回位置和标志到 RCX/R11；它不自动切换 RSP，也不保存完整现场。Linux 入口软件保存用户栈指针、切换当前线程内核栈并构造 pt_regs。具体服务由内核读取调用号后分派；返回可以走 SYSRETQ 或适用的 IRETQ 路径。

## 2. 寄存器名称与 ABI

寄存器由 x86-64 架构定义；Linux x86-64 syscall ABI 规定这次交互怎样使用它们。

| 内容 | 位置 |
| --- | --- |
| 系统调用号，执行 syscall 之前 | RAX |
| 六个参数，按顺序 | RDI、RSI、RDX、R10、R8、R9 |
| 返回结果 | RAX |

实际 write 实验中，syscall 指令前 RAX=1（write 编号）、RDI=1（标准输出 fd）、RSI=字符数组地址、RDX=13。返回后 RAX=13。两个 1 的含义不同，RSI 装地址而非整段字符。普通 C 调用约定的第四个整数/指针参数使用 RCX，与系统调用不同。

GDB 的 catch syscall 入口停点不等于停在 syscall 指令执行前，不能据此假设入口停点的 RAX 一定仍是调用号。此次指令前数值来自对实际 libc syscall 指令设置断点后的观察。

## 3. 两个观察工具

- GDB：观察 add 的调用栈、返回值，以及 libc 指令和寄存器。已使用 break、run、bt、finish、next、catch syscall write、info registers、x/s、disassemble 和 stepi。
- strace：记录系统调用及其参数、结果与信号，不直接记录普通 add 调用。-e trace=... 筛选展示的系统调用，-y 补充 fd 对应目标，-o 将跟踪记录保存到文件。
- strace 默认向标准错误输出自己的记录；程序的标准输出和跟踪日志是两个来源。

学生已正确复述 main → libc → syscall → 内核 → libc → main；入口机制属于讲解与源码阅读，指令/边界部分有助手实际 GDB 运行证据。

## 4. write、errno 和进程退出

正常 stdout 实验的真实记录：

```text
write(1, "hello kernel\n", 13) = 13
exit_group(0) = ?
+++ exited with 0 +++
```

字符数组共 14 字节，sizeof(text)-1 排除结尾 NUL。write 按地址和长度取字节，不寻找 NUL。早期误用字符指针的 sizeof，实际请求并完整写入了 7 字节；这属于请求长度错误，不能叫短写。

无效 fd 变体的真实记录：

```text
write(-1, "hello kernel\n", 13) = -1 EBADF (Bad file descriptor)
write(2, "write: Bad file descriptor\n", 27) = 27
exit_group(-1) = ?
+++ exited with 255 +++
```

在该 ABI 下，内核原始错误为负错误码；libc 将它转换为 -1 并设置 errno。strace 会解码显示错误名称，不能由日志格式推断内核直接写了用户态 errno。

perror("write") 读取当前 errno，将前缀与错误说明输出到标准错误。它本身不退出程序。main 返回 -1 时，外部观察到低 8 位退出状态 255；写入返回值、errno 和退出状态分别表示不同的信息。

C 库 exit 做退出清理，包括注册的退出处理函数和标准 I/O 流刷新；当前 libc 的进程终止路径使用 exit_group。Linux 的原始 exit 系统调用终止调用线程，exit_group 终止进程线程组。成功退出不再返回用户代码。

## 5. fd 与重定向

学生亲手运行同一程序的终端输出和 >stdout.txt 对照，报告两次 write 的 fd 都为 1，目标分别为终端与文件。

fd 是进程文件描述符表中的索引。shell 在程序开始前安排重定向，改变标准输出 fd 1 所连接的目标；> 不会成为 write 的参数。标准错误 fd 2 未随这个 > 改变，因此 strace 的记录仍能在终端看到。

此对照按学生实际观察记录，未声称助手独立复测。fd 数字不是文件的永久身份；复制整数与复制资源的区别留给下一练习继续验证。

## 6. 自己管理 open → write → close

学生实现 file_write.c，打开 owned-output.txt，用 O_WRONLY | O_CREAT | O_TRUNC 与创建权限 0644。0644 是八进制创建权限，还受 umask 等创建规则影响。open 成功返回非负 fd；失败返回 -1；不能假设 fd 总是 3。

write 的结果保存在 ssize_t 中，检查错误与短写。成功打开后的各条退出路径均关闭 fd 并检查关闭结果。write 错误先 perror 再清理，避免清理操作改变 errno 后才报告原错误。本练习将短写作为失败，完整重试循环留后续 I/O 课。

修正过的关键问题：

- 写入失败和短写直接 return 会跳过显式 close；进程退出后内核会关闭遗留 fd，但练习要求在正常与失败路径都明确清理。
- return close(fd) 会在关闭成功时把前面的写入失败误报为进程成功。
- 失败分支可以记录 close 错误后保留原来的非零 return；正常收尾关闭失败则必须提前返回非零，不能继续落到 return 0。
- close 成功不保证文件内容已经持久化到存储设备；本轮验证的是接口结果与可读出的文件内容。

验收证据：

| 场景 | 实测结果 | 验证阶段 |
| --- | --- | --- |
| 正常文件写入 | 11 字节 hello file 加换行；close=0；退出 0 | 最终实现 |
| 模拟关闭失败 | strace 对目标 close 注入 EIO；输出错误；退出 1 | 最终实现 |
| 打开失败 | 同名目录触发 EISDIR；退出 255 | 此前隔离测试 |
| 写入失败 | /dev/full 触发 ENOSPC；close=0；退出 255 | 此前隔离测试 |
| 真实短写 | 测试子进程的 RLIMIT_FSIZE=5；write 请求 11 返回 5；文件为 hello；close=0；退出 1 | 此前隔离测试 |

后面三项运行后，学生补充了 close 错误检查并修正正常关闭失败的返回状态；最终版本对这些分支作代码核对，没有重复运行全部案例。关闭错误注入只验证返回值处理，不代表真实磁盘故障，也不验证真实 close 失败时描述符的释放时机。

本机完整日志位于被 Git 忽略的 build/f03-os00：

- syscalls-check.trace、syscalls-invalid-fd.trace。
- gdb-boundaries.log、gdb-syscall-registers.log、gdb-write-instructions.log、gdb-syscall-instruction.log。
- file-write-check-scp4_j1z/：此前三条失败/短写路径及正常路径。
- file-write-accepted-aeqiuv96/：最终严格构建、正常路径和模拟 close 失败。

本次下班整理沿用已有验证；只清理已完成源码中的过期 TODO/占位注释，没有改动控制逻辑，也没有重新扩大测试。

## 7. 下次从这里继续

打开 src/fd_reuse.c，按 FD_REUSE.md 完成：

1. 打开第一个文件，保存 first 和整数副本 saved。
2. 关闭 first 并检查，再手动设 first=-1。
3. 打开第二个文件，打印 first、saved、second。
4. 关闭 second 并检查。
5. 结合 strace -y 解释数字复用与资源身份的关系。

VS Code：F1 → Tasks: Run Task → F03 OS00: build and trace fd reuse。
该任务从 build/f03-os00 运行，两个实验文件位于该目录。当前只有脚手架，构建入口已检查，学生逻辑尚未实现，不能标记通过。

## 参考资料

- [write(2)](https://man7.org/linux/man-pages/man2/write.2.html)、[syscall(2)](https://man7.org/linux/man-pages/man2/syscall.2.html)、[库封装与系统调用](https://man7.org/linux/man-pages/man2/syscalls.2.html)。
- [open(2)](https://man7.org/linux/man-pages/man2/open.2.html)、[close(2)](https://man7.org/linux/man-pages/man2/close.2.html)、[exit(3)](https://man7.org/linux/man-pages/man3/exit.3.html)。
- [strace 手册](https://man7.org/linux/man-pages/man1/strace.1.html)、[GDB catchpoints](https://sourceware.org/gdb/current/onlinedocs/gdb.html/Set-Catchpoints.html)。
- [Linux v5.14 x86-64 入口](https://github.com/torvalds/linux/blob/v5.14/arch/x86/entry/entry_64.S)、[分派代码](https://github.com/torvalds/linux/blob/v5.14/arch/x86/entry/common.c)、[write 实现](https://github.com/torvalds/linux/blob/v5.14/fs/read_write.c)。
