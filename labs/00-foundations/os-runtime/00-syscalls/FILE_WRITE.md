# OS00 后续练习：自己管理文件描述符

日期：2026-11-02。状态：学生实现与本轮验收已完成，证据和范围见文末。

## 目标

沿着已有的 syscall / fd 认识，手写一次 open → write → close。此处只建立文件描述符生命周期；fd 表共享、dup2、fork/exec、文件系统与持久化在后续单元深入。

源文件：src/file_write.c。独立目标：f03-os00-file-write。
VS Code 任务：F1 → Tasks: Run Task → F03 OS00: build and trace file write。
任务从仓库根目录构建，再进入 build/f03-os00 运行，故相对路径 owned-output.txt 位于该构建目录。

## 接口与语法

需要 fcntl.h 声明 open 和打开标志；unistd.h 声明 write、close；stdio.h 提供 perror 等输出函数。

示例调用：

```c
int fd = open("owned-output.txt",
              O_WRONLY | O_CREAT | O_TRUNC, 0644);
```

- O_WRONLY：只写；O_CREAT：不存在时创建；O_TRUNC：成功打开已有普通文件时清空内容。
- 单竖线 | 是按位或，用于组合这些标志，不是逻辑或 ||。
- 0644 是八进制的创建权限：所有者读写、所属组和其他用户只读；实际新文件权限还受 umask 等创建规则影响。已有文件权限不会因这个参数重新设置。
- open 成功返回非负 int，失败返回 -1 并设置 errno；不要假设成功值一定是 3。
- close(fd) 成功返回 0，失败返回 -1；关闭释放描述符，不删除文件。此 Linux 练习中不重试 close。

## 学生实现要求

1. 在 main 中打开 owned-output.txt，失败时立即 perror("open")，返回非零。
2. 定义字符数组，内容是 hello file 加一个换行；向刚得到的 fd 写入 sizeof(array)-1 字节。
3. 用 ssize_t 保存 write 结果，检查 -1 与短写。本步短写算失败，不要求重试循环。
4. write 返回 -1 时先 perror("write")，再清理，避免清理操作改变 errno 后才报告原错误。短写不以 perror 报告，因为它不一定设置 errno。
5. open 成功后的每条退出路径都 close 一次并检查其结果；关闭失败时 perror("close") 并保持失败状态。open 失败不 close 无效 fd。
6. 仅在完整写入且关闭成功时返回 0。

## 验收

实现后检查严格 C11 构建、退出状态、文件精确字节 hello file\n（11 字节），以及该目标文件的打开、写入、关闭记录。
打开失败实际采用目标路径为同名目录的 EISDIR 变体；未使用不存在父目录的变体。

strace 任务包含 open、openat、write、close、exit_group，并用 -y 显示 fd 目标。源码 open 在当前 glibc/Linux 上通常显示为 openat；启动阶段也会打开动态库，检查时按目标文件路径识别本练习的调用，不把启动调用混入计数。

原占位代码已由学生实现替换。实际构建、运行及失败路径证据见文末。

## 来源

- [open(2)](https://man7.org/linux/man-pages/man2/open.2.html)
- [write(2)](https://man7.org/linux/man-pages/man2/write.2.html)
- [close(2)](https://man7.org/linux/man-pages/man2/close.2.html)

## 2026-11-02 验收记录

open → write → close 本轮练习已通过。最终源文件的写失败、短写和正常收尾路径均调用 close 并检查结果；写入失败不会被关闭成功覆盖成成功退出。

- 最终版本严格 C11 构建通过，正常文件字节为 hello file\n（11 字节），write 返回 11、close 返回 0、进程退出 0。
- 最终版本通过 strace 对目标 close 注入 EIO，实际报告 close: Input/output error 并退出 1。此项只验证错误处理，是模拟错误，不是实际磁盘故障或真实 close 资源释放语义实验。
- 沿用此前隔离实验结果：同名目录触发 open 的 EISDIR（退出 255）；/dev/full 触发 write 的 ENOSPC，随后关闭并退出 255；仅对子进程设 RLIMIT_FSIZE=5，真实 write 请求 11 返回 5，文件为 hello，随后关闭并退出 1。此后补充 close 错误检查并修正正常关闭失败的返回状态；最终版本对这些分支作代码核对，未重复运行这三种案例。
- 最终构建/正常/关闭错误证据：build/f03-os00/file-write-accepted-aeqiuv96/。
- 打开失败/写入失败/真实短写证据：build/f03-os00/file-write-check-scp4_j1z/。
- 下班整理仅清理已完成代码中的过期 TODO/占位注释，未修改控制逻辑；perror 前缀 opern file 保留，不影响上述验收。

下一步：[文件描述符的复用](FD_REUSE.md)，新练习尚待实现。
