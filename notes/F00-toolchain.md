# F00：编译与运行证据链

状态：进行中  
日期：2026-09-10  
环境：Rocky Linux 9.8 x86_64、GCC Toolset 14、Node v24.20.0

## 当前模型

```text
main.cc
  → 预处理 main.ii
  → 编译为汇编 main.s
  → 汇编为可重定位目标文件 main.o
  → 链接为 runtime-smoke
  → 动态加载器装载依赖
  → _start
  → main
```

`g++` 是编译器驱动程序，会协调预处理器、编译器、汇编器和链接器。`.o` 已含机器码，但仍有未解析符号和重定位信息，也没有供操作系统启动的程序入口及装载描述。

## 文件证据

| 文件 | 大小 | 识别结果 |
| --- | ---: | --- |
| `main.ii` | 1.1 MB | 展开头文件后的 C++ 文本 |
| `main.s` | 301 KB | 汇编文本，包含调试相关指令 |
| `main.o` | 130 KB | ELF64 `REL`，入口为 0，没有 Program Header |
| `runtime-smoke` | 91 KB | 动态链接的 ELF64 `EXEC`，带 debug info |

可执行文件的入口地址与 `_start` 符号地址一致，而不是 `main`。动态加载器处理依赖和重定位后进入 `_start`，C/C++ 运行时再调用 `main`。

Section 是链接期按用途划分的内容；Segment 是装载期的内存映射和权限方案。当前只保留这一级认识，遇到实际问题时再用 `readelf` 回查。

## GDB 证据

```text
#0 print_task(context=0x7fffffffd580)
#1 run_callback(callback=print_task, context=0x7fffffffd580)
#2 main()
```

- `#0` 是当前线程调用栈的栈顶；`frame` 只切换观察对象，不改变执行位置。
- 函数指针指向 `print_task`，`context` 在两层调用中保持同一个地址。
- `(Task*)context` 只是按 `Task` 的内存布局解释地址，不创建对象或转移所有权。
- `Task` 是 `main` 的栈对象，回调只临时借用其地址。
- 已练习 `break`、`run`、`bt`、`frame`、`print`、`next`、`finish` 和 `continue`。

## strace 与 perf 证据

```text
write(1, task-output, 30) = 30
exit_group(0) = ?
exited with 0
```

`strace -c` 共观察到 118 次系统调用。数量较多的是 `openat`、`mmap` 和 `mprotect`，主要来自动态加载器搜索、映射动态库并设置内存权限。失败的路径探测不等于程序运行失败。

`perf stat` 观察到约 2.54 ms task-clock、2 次上下文切换、0 次 CPU 迁移和 125 次缺页。程序过短，数据只用于认识计数器，不能作为可靠性能结论。

## 下一步

完成 CMake、Make 与增量构建实验，再进行 F00 简短验收。大型工程阶段切换到 Ninja；当前 Rocky 软件源未提供 Ninja，不为单文件实验中断主线。
