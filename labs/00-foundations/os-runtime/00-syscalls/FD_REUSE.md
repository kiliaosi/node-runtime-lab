# OS00 后续练习：文件描述符的复用

布置日期：2026-11-02。状态：学生实现已提交，正常运行与两次 open 分别失败的路径已验证。

## 本次问题

fd 是进程文件描述符表中的整数索引。复制 int 只复制编号，不创建新的描述符，也不延长原文件的打开生命周期。close(fd) 释放描述符，但不会自动修改 C 局部变量的值。Linux open 分配当前最小的可用描述符，因此关闭后的编号可以被后续 open 复用。

## 学生任务

在 src/fd_reuse.c 中实现 main：

1. 用 O_WRONLY | O_CREAT | O_TRUNC 和创建权限 0644 打开 reuse-first.txt，保存到 int first；失败则 perror 并返回非零。
2. 保存 int saved = first。关闭 first 并检查结果；失败则 perror 并返回非零。成功后手动 first = -1。
3. 用同样参数打开 reuse-second.txt，保存到 int second，并检查失败。
4. 用 printf 和 %d 打印 first、saved、second。
5. 通过 second 关闭本次有效描述符一次，检查结果，失败返回非零，成功返回 0。

saved 只是用于观察的整数副本；不对它再做 close。此步只观察编号与打开/关闭记录，两个新文件预期保持为空。

## 运行与验收

F1 → Tasks: Run Task → F03 OS00: build and trace fd reuse。

独立构建目标 f03-os00-fd-reuse。任务在 build/f03-os00 中运行，两个输出文件都位于该构建目录。

验收：严格构建通过；每次成功打开都正确关闭；first 打印 -1，saved 保留第一次取得的编号。对照第二次 open 的编号及 strace -y 中的文件路径，解释相同数字能否代表不同的文件。不硬编码 fd=3，不把具体编号作为所有运行环境的保证。

正常运行时，第二次 open 复用了第一次的描述符编号。strace -y 显示两个 open 对应的是不同文件。两次 open 的失败路径均用同名目录触发 EISDIR，程序报告失败并退出。旧编号在复用后可能访问第二个文件，此处仅做推演。

## 来源

- [open(2)：返回最小可用文件描述符](https://man7.org/linux/man-pages/man2/open.2.html)
- [close(2)：释放描述符并允许复用](https://man7.org/linux/man-pages/man2/close.2.html)
