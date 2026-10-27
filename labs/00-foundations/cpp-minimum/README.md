# F02 C++ 最小集：对象生命周期与所有权

更新日期：2026-10-27
状态：四组 Trace 实验与六组 File 实验已通过严格构建和运行；F02 整体仍在进行中，移动赋值待实现。

课程范围见 [CURRICULUM.md](../../../docs/CURRICULUM.md)，总进度见 [PROGRESS.md](../../../PROGRESS.md)。
笔记：[引用与 unique_ptr](../../../notes/F02-cpp-minimum-part1.md)、[File RAII 与资源转移](../../../notes/F02-cpp-minimum-part2.md)。

## Trace：生命周期与智能指针

源码：[src/main.cc](src/main.cc)，构建目标：f02-lifetime。

| 函数 | 已验证的行为 |
| --- | --- |
| test_scope | 内层作用域结束时析构，destroy 在 C 之前 |
| test_unique | 打印 B 后提前 return，仍然析构，不输出 C |
| test_move | q 接管 p 的所有权；p 为空，q 非空；内层结束时销毁 Trace |
| custom_trace / test_transfer | 按值形参接管 unique_ptr；调用后的输出之前已销毁 Trace，调用者 item 为空 |

## File：真实文件资源

源码：[src/file_raii.cc](src/file_raii.cc)，构建目标：f02-file-raii。
接口、完成条件与下一步任务见 [FILE_RAII.md](FILE_RAII.md)。

当前练习使用 unique_ptr 管理 File 对象，File 再管理 FILE*。
主程序依次调用六个测试：

| 函数 | 已验证的行为 |
| --- | --- |
| test_file_scope | 正常离开作用域时关闭文件 |
| test_file_return | 提前 return 时关闭文件 |
| test_file_failure | 打开目录失败，valid/write 均返回 false，空状态安全析构 |
| test_file_borrow | File& 借用后，原所有者仍可以继续写入 |
| test_file_move | File 的移动构造接管句柄并清空来源；两个 unique_ptr 本身仍非空 |
| test_file_transfer | write_owned(File) 的按值参数接管句柄并关闭文件；外层智能指针非空，其 File 已为空 |

write_owned 当前实现为 File 的成员函数，调用形式为 file->write_owned(std::move(*file))。
全局 write_message(File&) 是借用辅助函数。

## 构建与运行

在 Rocky Linux 的仓库根目录执行：

```bash
cd /root/workspace/node-runtime-lab
source scripts/activate-lab.sh
cmake -S labs/00-foundations/cpp-minimum -B build/f02-cpp-minimum -DCMAKE_BUILD_TYPE=Debug
cmake --build build/f02-cpp-minimum --clean-first
timeout 5s ./build/f02-cpp-minimum/f02-lifetime
timeout 5s ./build/f02-cpp-minimum/f02-file-raii
```

编译成功后再运行。本实验独立配置，不依赖 libuv；采用 C++20，关闭语言扩展。
GCC/Clang 使用 -Wall -Wextra -Wpedantic -Werror，MSVC 配置 /W4 /WX /permissive-。
本轮实际验证环境为 Rocky Linux / GNU C++ 14.2.1；未在 Windows 上验证此实验。

## 在 VS Code 终端中查看输出

连接 172.20.14.144 的 Remote SSH 窗口，以仓库根目录作为工作区。保存文件后：

1. 按 F1，运行 Tasks: Run Task。
2. 选择 F02: build and run File RAII。
3. 在自动显示的终端中查看编译器诊断和程序输出。

Trace 实验对应 F02: build and run lifetime。
当前机器的 Ctrl+Shift+B 被搜狗输入法拦截，使用上面的命令面板入口即可。

[任务配置](../../../.vscode/tasks.json) 先激活工具链、配置和编译，再运行程序。
命令用 && 连接，编译失败时停止，不运行旧可执行文件。
任务固定从仓库根目录运行，终端保留输出，GCC problem matcher 提供报错定位。
这两个任务用于 Linux Remote SSH 环境。

## File 的实际运行输出

以下保留程序当前的原始输出；部分 printf 没有换行，故相邻文字连在一起。
源码没有成功打开/关闭的日志，资源关闭通过源码和 strace 共同确认。

```text
normal
after scopeearly
before returnafter return
failure
0
0
after failure
after borrowafter borrow test
0
1
1
after move test
1
1
0
after transfer test
```

生成文件均在 build/f02-cpp-minimum/ 下，build/ 已被 Git 忽略：

| 文件 | 精确内容 | 字节数 |
| --- | --- | --- |
| raii-normal.txt | normal\n | 7 |
| raii-early.txt | early\n | 6 |
| raii-borrow.txt | borrowed\nowner\n | 15 |
| raii-move.txt | moved\n | 6 |
| raii-transfer.txt | owned\n | 6 |

表中的 \n 表示一个换行字节。
严格构建无警告，两程序退出码均为 0。系统调用记录确认五个练习文件各打开、关闭一次；
transfer 文件在调用后的结果输出之前已关闭。未执行 sanitizer、分配失败注入或故意的 double-free/use-after-free 实验。

## 下次从这里继续

实现 File& operator=(File&& other) noexcept 和 test_file_move_assign()。
重点是目标对象已存在时，先处理自移动、关闭旧资源，再接管新资源并清空来源。
具体任务见 [移动赋值练习](FILE_RAII.md#下一步移动赋值待完成)。
右值引用和赋值运算符重载已经讲解，移动赋值代码尚未实现、尚未验收。
