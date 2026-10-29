# F02 C++ 最小集（二）：真实文件资源的 RAII、借用与转移

日期：2026-10-27
学习范围：2026-10-23 至 2026-10-27。
状态：File RAII 六组实验已验证；F02 整体仍在进行中。

接续：[第一部分](F02-cpp-minimum-part1.md)；总进度见 [PROGRESS.md](../PROGRESS.md)。
代码：[file_raii.cc](../labs/00-foundations/cpp-minimum/src/file_raii.cc)；练习要求：[FILE_RAII.md](../labs/00-foundations/cpp-minimum/FILE_RAII.md)；运行入口：[实验 README](../labs/00-foundations/cpp-minimum/README.md)。

## 1. 从 Trace 日志走到真实资源

本轮手写 `File`，管理 `std::fopen(path, "w")` 返回的 `std::FILE*`。
`valid() const` 判断是否持有有效流；`write` 判空后调用 `std::fputs`，以返回值是否为 `EOF` 判断该次写入结果。
析构先判空，有资源时调用 `fclose`。流用 `fclose` 关闭，不用 `free` 或 `delete`。

当前实验使用三层管理关系：

```text
局部 unique_ptr<File> -> 动态 File 对象 -> FILE* 文件资源
```

- `unique_ptr` 负责销毁 `File` 对象及其动态存储。
- `File` 析构负责关闭它持有的文件资源。
- 智能指针非空不代表文件打开成功，仍要检查 `File::valid()`。

这里的“三层”是责任关系，不是本轮测得了固定三次内存分配。
正常离开作用域和提前 return 都会触发局部所有者的析构。
打开失败时仍有合法的 `File` 包装对象，但内部流为空，写入返回 false，析构不关闭空流。

`"w"` 会截断已有文件，练习文件固定写入仓库的 `build/f02-cpp-minimum`，不写用户其他文件。
相对路径依赖运行目录；父目录须事先存在，`fopen` 不创建目录。

## 2. 六组手写实验

| 函数 | 核心观察 |
| --- | --- |
| `test_file_scope` | 正常写入 `normal\n`，离开内层作用域后资源已关闭 |
| `test_file_return` | 写入 `early\n` 后提前 return，调用方继续打印 `after return` |
| `test_file_failure` | 尝试用写模式打开目录 `.`，valid/write 都为 false，无空流关闭 |
| `test_file_borrow` | 引用形参借用同一 File，返回后所有者还能继续写 |
| `test_file_move` | 调用自定义 File 移动构造，源 File 失去资源，目标继续写 |
| `test_file_transfer` | 按值参数承接文件资源，调用完成后参数负责关闭 |

本轮源代码在成功打开和成功关闭时没有日志，不能补写不存在的 open/close 输出。
部分 `printf` 没有换行，因此相邻日志会连在同一行；资源验证同时检查文件内容与系统调用。

## 3. 引用借用与复制保护

全局函数的实际接口为：

```cpp
bool write_message(File& file);
```

形参是同一个 `File` 的别名，不取得资源所有权，不会在函数返回时销毁调用者对象。
测试中的 `f` 是 `unique_ptr<File>`，用 `write_message(*f)` 传入它管理的 File。
函数写入 `borrowed\n` 并返回写入结果；返回后所有者再写入 `owner\n`，最终只关闭一次。

复制保护始终保留：

```cpp
File(const File&) = delete;
File& operator=(const File&) = delete;
```

如果把引用参数改为按值参数，并直接传入已有 File 左值，将尝试调用已经删除的拷贝构造函数。
这是阻止两个独立对象分别关闭同一流的编译期约束；按值参数也可以通过移动构造完成初始化。
以上是语义说明，本轮没有保留临时复制实验的编译输出，也没有删除保护去运行重复关闭。

## 4. File 自身的移动构造

```cpp
File(File&& other) noexcept {
  this->file_ = other.file_;
  other.file_ = nullptr;
}
```

`File&&` 在这里是移动构造函数的引用形参；`noexcept` 表达该构造函数不抛异常的承诺。
本次只做句柄接管和源指针置空，不重新打开文件，不复制文件内容。
置空是这个自定义构造函数明确执行的行为，不是 `std::move` 自动替所有类型完成的动作。

测试保留学生使用 `unique_ptr` 的形式：

```cpp
std::unique_ptr<File> second = std::make_unique<File>(std::move(*first));
```

- `*first` 是已有 File，`std::move(*first)` 使新 File 用移动构造接管句柄。
- first、second 两个智能指针都仍非空，管理的是两个不同的 File 对象。
- 源 File 的内部流为空，`first->valid()` 安全返回 false；打开成功的目标返回 true。
- 两个 File 都会析构，只有仍持有句柄的那个负责关闭。

这与 `std::unique_ptr<File> q = std::move(p)` 这种直接移动智能指针的写法不同：后者转交同一个动态对象，源智能指针为空，File 对象本身没有被移动构造。
本次测试输出源 valid 为 0、目标 valid 为 1、写入结果为 1，并写出 `moved\n`。

## 5. File 成员函数中的按值接管

实际保留的设计是 File 的 public 非 static 成员，不是类外函数：

```cpp
bool write_owned(File file);
```

测试中的调用为：

```cpp
bool result = file->write_owned(std::move(*file));
```

外层 `file` 是 `unique_ptr<File>`；成员函数的同名形参 `file` 是一个新的 File 对象。
参数通过移动接管资源；被调用成员函数的原对象仍然存活，只是已处于空资源状态。
当前函数体只使用按值参数，先检查 `file.valid()`，再 `return file.write("owned\n")`，不依赖 this 原来的资源，因此这种调用合法。

调用完成后，参数对象已关闭文件，外层智能指针仍非空，内部 File 的 valid 为 false。
本次三项输出依次为：写入结果 1、智能指针非空 1、原 File 有效性 0。
系统调用跟踪确认 transfer 文件在调用后的结果输出之前已经关闭。

## 6. 本轮纠正与自查

- `fopen` 的模式写成字符串 `"w"`，不能传字符 `'w'`。
- `EOF` 是宏，不写 `std::EOF`；使用智能指针和移动时直接包含 `<memory>`、`<utility>`。
- `std::make_unique<File>(...)` 需要明确本次创建的类型；默认空 unique_ptr 不会创建 File。
- File 对象和引用使用 `.`，智能指针使用 `->`；形参 `other` 是引用，写 `other.file_`。
- 需要移动 File 时传 `std::move(*first)`，不能把 `unique_ptr<File>` 当成 File 构造参数。
- 返回 bool 的成功路径也要返回结果；不能只执行 write 后落到函数末尾。
- 输出比较式需要括号，例如 `std::cout << (file != nullptr)`；直接输出 bool 也可以。
- 在 return 后追加日志不会执行；应在调用方观察函数返回后的时序。
- 声明、定义、main 调用和观察项应一起核对；函数未被调用时，编译成功不能证明实验执行过。
- 保存后检查真正参与远程构建的文件；本轮曾出现编辑反馈与远端落盘内容不一致，需以实际构建文件为准。

本轮只入门了 `&&`、`noexcept` 和 `operator=` 的对应含义，没有展开右值引用实现、完美转发等专题。

## 7. 实测输出与文件内容

以下是当前六组实验的实际 stdout；保留原有缺换行造成的拼接：

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

| 输出文件（均位于 build/f02-cpp-minimum） | 实际内容 | 字节数 |
| --- | --- | --- |
| `raii-normal.txt` | `normal\n` | 7 |
| `raii-early.txt` | `early\n` | 6 |
| `raii-borrow.txt` | `borrowed\nowner\n` | 15 |
| `raii-move.txt` | `moved\n` | 6 |
| `raii-transfer.txt` | `owned\n` | 6 |

验证环境为 Rocky Linux、GNU C++ 14.2.1，严格参数为 `-std=c++20 -Wall -Wextra -Wpedantic -Werror`。
2026-10-27 提交前对两个目标重新构建与运行，均通过，无警告，进程退出 0；strace 确认这五个输出文件各成功打开、关闭一次。
没有运行 sanitizer、没有故意运行 double-free/use-after-free，也没有做分配失败注入，不能写成已覆盖这些检查。

## 8. 亲眼观察编译与运行

在 VS Code Remote SSH 窗口打开仓库根目录，用：
`F1 -> Tasks: Run Task -> F02: build and run File RAII`。
本机 Ctrl+Shift+B 曾被搜狗输入法拦截，因此使用 F1 任务入口。

任务以仓库为工作目录，激活工具链、配置 CMake、构建 `f02-file-raii` 后再运行；编译失败不会运行旧程序。
可在仓库根目录使用对应命令：

```bash
source scripts/activate-lab.sh
cmake -S labs/00-foundations/cpp-minimum -B build/f02-cpp-minimum -DCMAKE_BUILD_TYPE=Debug
cmake --build build/f02-cpp-minimum --target f02-file-raii --verbose
./build/f02-cpp-minimum/f02-file-raii
```

## 9. 下次从移动赋值继续

下一步已布置，尚未实现或验收：`File& operator=(File&& other) noexcept` 与 `test_file_move_assign`。
使用两个已经存在、各持有不同文件的直接局部 File 对象，区分移动构造和移动赋值，不再嵌套智能指针。
预期旧文件保留 `old\n`，新文件内容为 `before\nafter\n`；后续检查旧资源及时关闭、接管后可继续写入及关闭责任唯一。
这只是下次的目标，不是本轮已取得的结果，此处不预先写完整实现。

F02 仍需按课程继续其他内容和综合验收。口头范围估计不是成绩或验收标准，本轮通过也不等于完整 C++ 已学完。


## 后续进展（2026-10-29）

移动赋值现已实现并完成课堂验证；另已推进共享所有权、模板、多态和 lambda，详见 [第三部分](F02-cpp-minimum-part3.md)。本篇保留截至 2026-10-27 的历史状态；无捕获 lambda 转函数指针仍是下次起点，F02 整体继续进行。
