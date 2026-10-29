# F02 练习二：用 File 管理真实文件资源

开始日期：2026-10-23
更新日期：2026-10-29
状态：七组 File 实验已在课堂验证，含移动赋值；F02 整体未完成。

源码：[src/file_raii.cc](src/file_raii.cc)；
构建、实际输出与文件内容见 [README](README.md)；
学习记录见 [F02 笔记（二）](../../../notes/F02-cpp-minimum-part2.md)。

## 目标与当前实现

File 拥有一个 std::FILE*，把打开和关闭文件的责任绑定到对象生命周期。
当前学生实现使用 std::unique_ptr<File> 管理 File，File 析构时关闭自己的文件。
这属于本轮已接受并验证的实现；也可以直接使用局部 File 对象。

当前接口包括：

```cpp
explicit File(const char* path);
~File();
bool valid() const;
bool write(const char* text);
File(File&& other) noexcept;
File(const File&) = delete;
File& operator=(const File&) = delete;
File& operator=(File&& other) noexcept;
bool write_owned(File file);
```

- public 成员可由外部调用；private 的 file_ 保存内部句柄，初始值为 nullptr。
- explicit 防止通过单参数构造函数进行隐式转换。
- valid() const 不改变对象的资源状态。
- 两个 = delete 禁止复制，避免两个对象同时负责关闭同一个句柄。
- 移动构造接管 other.file_ 并把它置空；不会再次打开文件。
- 移动赋值先处理目标旧资源，再接管来源并清空来源。

## 使用的 C 文件接口

接口来自 <cstdio>，资源类型是 std::FILE*。

| 接口 | 行为 |
| --- | --- |
| std::fopen(path, "w") | 打开或创建文件；成功返回 FILE*，失败返回 nullptr |
| std::fputs(text, file_) | 写入 C 字符串；不会自动追加换行；EOF 表示失败 |
| std::fclose(file_) | 关闭有效文件流；返回 0 表示成功，EOF 表示失败 |

"w" 会清空已有文件。练习文件统一放在 build/f02-cpp-minimum/ 下。
FILE* 使用 fclose 关闭，不使用 free 或 delete。调用 fclose 后不再使用或重试关闭该流。

## 已完成的行为

1. 构造函数保存 fopen 的结果；打开失败仍得到一个内部句柄为空的 File。
2. valid() 根据内部 file_ 判断文件是否有效，不能仅判断外层 unique_ptr 是否非空。
3. write() 先判空，再返回 fputs 的成功/失败结果；不负责关闭文件。
4. 析构函数判空后关闭资源，空对象可以安全析构。
5. File& 借用允许调用者在函数返回后继续使用原文件。
6. File 的移动构造和按值函数参数能够转移文件管理权，来源成为空 File。

初始任务建议通过 open/close 日志观察生命周期；当前源码没有这些成功日志，部分输出也未换行。
本轮按实际源码、输出、文件字节和 strace 的打开/写入/关闭顺序完成验证，没有把最初的预期日志当成运行结果。

## 七个测试及验收点

| 测试 | 验收点 |
| --- | --- |
| test_file_scope | 正常离开作用域关闭文件，raii-normal.txt 为 normal\n |
| test_file_return | 提前 return 仍关闭文件，raii-early.txt 为 early\n |
| test_file_failure | 在 Linux 上以 "w" 打开目录 "." 失败，valid/write 都为 0，无无效关闭 |
| test_file_borrow | write_message(File&) 写入 borrowed\n 后，所有者继续写 owner\n |
| test_file_move | make_unique<File>(std::move(*first)) 调用 File 移动构造；两次 valid 和写入结果为 0、1、1 |
| test_file_transfer | file->write_owned(std::move(*file)) 接管资源；结果、外层指针非空、内部有效性为 1、1、0 |
| test_file_move_assign | *target = std::move(*source)；目标/来源 valid 和继续写入结果为 1、0、1 |

write_message 是类外函数，参数 File& 使用点号访问成员。
write_owned 当前是 File 的成员函数，但其按值形参仍是独立的 File 对象，使用点号访问成员。
test_file_transfer 中的 file 则是智能指针，通过箭头访问它管理的 File。

七个测试均已在 main 中调用。输出、生成文件和构建步骤以 README 为准。

## 移动赋值（已完成）

已实现接口：

```cpp
File& operator=(File&& other) noexcept;
```

实现先检查 this == &other；若自身有资源则关闭旧流，再接管 other.file_，
把来源置为 nullptr，并返回 *this。自移动保护已检查源码，本轮未单独运行自移动场景。

当前学生测试使用两个 unique_ptr<File>：

```cpp
std::unique_ptr<File> target =
    std::make_unique<File>("build/f02-cpp-minimum/raii-assign-old.txt");
std::unique_ptr<File> source =
    std::make_unique<File>("build/f02-cpp-minimum/raii-assign-new.txt");
```

检查打开成功后，target 写 old\n，source 写 before\n，然后执行：

```cpp
*target = std::move(*source);
```

这是对已存在 File 的移动赋值。target/source 仍管理各自原来的 File 对象，
资源从 source 内部转交给 target；来源 File 为空，但 source 智能指针仍非空。

直接写 target = std::move(source) 则转移 unique_ptr 自己的所有权，source 会变为空指针。
本轮曾在这种错误写法后调用 source->valid() 并观察到 SIGSEGV，随后改为移动 File 并验证成功。
这次错误不是 double-free/use-after-free 实验，也不能替代课程规定的资源错误验收。

测试结果：target->valid() 为 1、source->valid() 为 0、目标继续写 after\n 成功为 1。
旧文件内容为 old\n，新文件内容为 before\nafter\n。
此前课堂 strace 确认旧流在赋值过程中关闭，新流由接管后的目标对象关闭，各关闭一次。

初始化新对象（如 File target = std::move(source)）使用移动构造；
对已存在对象执行赋值才调用移动赋值运算符。

2026-10-29 下班提交按用户要求不重复构建或运行。后续课程入口见 [README](README.md#下次从这里继续)。
