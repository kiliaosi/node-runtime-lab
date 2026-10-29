# F02 C++ 最小集：资源、所有权、模板与多态

更新日期：2026-10-29
状态：main.cc 的十组实验和 file_raii.cc 的七组实验已在课堂验证；F02 整体仍在进行中。

课程范围见 [CURRICULUM.md](../../../docs/CURRICULUM.md)，总进度见 [PROGRESS.md](../../../PROGRESS.md)。
笔记：[引用与 unique_ptr](../../../notes/F02-cpp-minimum-part1.md)、[File RAII](../../../notes/F02-cpp-minimum-part2.md)、[移动赋值到 lambda 捕获](../../../notes/F02-cpp-minimum-part3.md)。

## 生命周期、模板、多态与 lambda

源码：[src/main.cc](src/main.cc)，构建目标：f02-lifetime。

| 函数 | 课堂验证结果 |
| --- | --- |
| test_scope | 内层作用域结束时析构，destroy 在 C 之前 |
| test_unique | 提前 return 仍析构，不输出 C |
| test_move | q 接管 p，p 为空、q 非空，内层结束销毁 Trace |
| test_transfer | custom_trace 按值接管 unique_ptr，调用者随后为空 |
| test_shared_ptr | p 计数依次 1、2；reset 后 p 为空且计数 0，q 离开内层时销毁对象 |
| test_template | larger 输出 7、2.5、8 |
| test_class_template | ValueBox<int>/float 输出 10、2.5、20 |
| test_polymorphism | 通过 const Operation& 调用 AddOperation/MultiplyOperation，输出 6、8 |
| test_polymorphic_owner | unique_ptr<Operation> 持有 MultiplyOperation，输出 12，先派生析构、后基类析构 |
| test_lambda | 无捕获输出 9、20；修改 num 后引用捕获输出 29，值捕获输出 19 |

Operation 是抽象类，run 为纯虚函数，析构为虚函数。
MultiplyOperation 的析构使用 override。ValueBox 构造函数使用 explicit 和成员初始化列表。
捕获部分直接写在 test_lambda 中；无单独的 test_capture 函数。

main.cc 新增部分的实际输出如下，早期四组 Trace 输出见第一部分笔记：

```text
shared_ptr test
create
1
2
1
0
after reset
destroy
after inner scope
template test
7
2.5
8
class template test
10
2.5
20
test calc
6
8
Operation destroy
MultiplyOperation destroy
Operation destroy
test_polymorphic_owner
12
MultiplyOperation destroy
Operation destroy
after owner scope
test_lambda
9
20
29
19
```

test_polymorphism 的两个局部对象按构造的逆序销毁：add 先销毁，mp 后销毁。
AddOperation 没有自定义析构日志，但仍会执行其析构，再执行有日志的基类析构。

## File：真实文件资源

源码：[src/file_raii.cc](src/file_raii.cc)，构建目标：f02-file-raii。
接口与实验说明见 [FILE_RAII.md](FILE_RAII.md)。

| 函数 | 课堂验证结果 |
| --- | --- |
| test_file_scope | 正常离开作用域时关闭文件 |
| test_file_return | 提前 return 时关闭文件 |
| test_file_failure | 打开目录失败，valid/write 均返回 false |
| test_file_borrow | File& 借用后，原所有者继续写入 |
| test_file_move | 移动构造转交句柄，源 File 为空，两个 unique_ptr 均非空 |
| test_file_transfer | write_owned(File) 接管并关闭资源；外层指针非空、内部 File 为空 |
| test_file_move_assign | *target = std::move(*source) 关闭目标旧资源并接管来源资源 |

write_owned 是 File 的成员函数，调用形式为 file->write_owned(std::move(*file))。
write_message(File&) 是类外借用函数。

实际 File 输出保留 printf 缺少换行造成的拼接：

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
1
0
1
after move assign test
```

生成文件均位于被 Git 忽略的 build/f02-cpp-minimum/：

| 文件 | 精确内容 | 字节数 |
| --- | --- | --- |
| raii-normal.txt | normal\n | 7 |
| raii-early.txt | early\n | 6 |
| raii-borrow.txt | borrowed\nowner\n | 15 |
| raii-move.txt | moved\n | 6 |
| raii-transfer.txt | owned\n | 6 |
| raii-assign-old.txt | old\n | 4 |
| raii-assign-new.txt | before\nafter\n | 13 |

表中的 \n 表示一个换行字节。课堂 strace 记录确认七个练习文件各打开/关闭一次，
移动赋值时关闭旧文件，接管的新文件最终由目标 File 关闭。
源码没有成功打开/关闭日志，不能把预期日志当成实测输出。

## 构建与运行

在 Rocky Linux 的仓库根目录执行，命令用 && 连接，编译失败不会运行旧程序：

```bash
cd /root/workspace/node-runtime-lab &&
source scripts/activate-lab.sh &&
cmake -S labs/00-foundations/cpp-minimum -B build/f02-cpp-minimum -DCMAKE_BUILD_TYPE=Debug &&
cmake --build build/f02-cpp-minimum &&
./build/f02-cpp-minimum/f02-lifetime &&
./build/f02-cpp-minimum/f02-file-raii
```

实验采用 C++20，关闭语言扩展；GCC/Clang 使用 -Wall -Wextra -Wpedantic -Werror。
实际课堂环境为 Rocky Linux / GNU C++ 14.2.1，未在 Windows 验证。
2026-10-29 下班保存按用户要求不重新构建/运行，上面的结果引用此前课堂验证。
未执行本阶段 sanitizer、分配失败注入、故意 double-free/use-after-free 或独立自移动测试。

## 在 VS Code 终端中查看输出

连接 172.20.14.144 的 Remote SSH 窗口，打开仓库根目录并保存文件。
按 F1 → Tasks: Run Task，选择 F02: build and run lifetime 或 F02: build and run File RAII。
[任务配置](../../../.vscode/tasks.json) 会激活工具链、构建并显示实际输出。
Ctrl+Shift+B 在当前机器与搜狗输入法冲突，使用 F1 入口。
当前机器的补全/格式化设置保留在本地工作区，不随本轮课程提交发布。

## 下次从这里继续

无捕获 lambda 转函数指针：在 test_lambda 的 add 定义后添加：

```cpp
int (*callback)(int, int) = add;
std::cout << callback(4, 5) << std::endl;
```

预期新增 9。这两行刚布置，尚未写入源码或验证。
之后继续 lambda 与 C 回调/context 桥接、V8 Local<T>/Maybe/MaybeLocal 阅读及资源错误修复实验。
