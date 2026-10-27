# F02 练习二：用 File 管理真实文件资源

开始日期：2026-10-23
更新日期：2026-10-27
状态：六组 File 实验已通过验证；移动赋值待实现，F02 整体未完成。

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
bool write_owned(File file);
```

- public 成员可由外部调用；private 的 file_ 保存内部句柄，初始值为 nullptr。
- explicit 防止通过单参数构造函数进行隐式转换。
- valid() const 不改变对象的资源状态。
- 两个 = delete 禁止复制，避免两个对象同时负责关闭同一个句柄。
- 移动构造接管 other.file_ 并把它置空；不会再次打开文件。
- 当前尚未实现移动赋值。

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

## 六个测试及验收点

| 测试 | 验收点 |
| --- | --- |
| test_file_scope | 正常离开作用域关闭文件，raii-normal.txt 为 normal\n |
| test_file_return | 提前 return 仍关闭文件，raii-early.txt 为 early\n |
| test_file_failure | 在 Linux 上以 "w" 打开目录 "." 失败，valid/write 都为 0，无无效关闭 |
| test_file_borrow | write_message(File&) 写入 borrowed\n 后，所有者继续写 owner\n |
| test_file_move | make_unique<File>(std::move(*first)) 调用 File 移动构造；两次 valid 和写入结果为 0、1、1 |
| test_file_transfer | file->write_owned(std::move(*file)) 接管资源；结果、外层指针非空、内部有效性为 1、1、0 |

write_message 是类外函数，参数 File& 使用点号访问成员。
write_owned 当前是 File 的成员函数，但其按值形参仍是独立的 File 对象，使用点号访问成员。
test_file_transfer 中的 file 则是智能指针，通过箭头访问它管理的 File。

六个测试均已在 main 中调用。输出、生成文件和构建步骤以 README 为准。

## 下一步：移动赋值（待完成）

本题让一个已经存在的 File 关闭旧文件，再接管另一个 File 的资源。
添加以下 public 成员函数，保留已有的禁止复制声明：

```cpp
File& operator=(File&& other) noexcept {
  // 学生自行实现
}
```

在 target = std::move(source) 中，this 指向 target，other 引用 source。
该表达式可以理解为调用 target.operator=(std::move(source))。

实现要求：

1. this == &other 时直接返回 *this，保留自移动时原来的资源。
2. 若自身 file_ 非空，先关闭自己原来的文件。
3. 接管 other.file_，随后把 other.file_ 置为 nullptr。
4. 返回 *this。赋值函数按惯例返回目标对象的引用。

直接覆盖旧的 file_ 会丢失旧文件句柄，造成资源泄漏。
先创建 target，再执行赋值；auto target = std::move(source) 是初始化新对象，会调用移动构造。

新增 test_file_move_assign()，直接使用局部 File：

```cpp
File target("build/f02-cpp-minimum/raii-assign-old.txt");
File source("build/f02-cpp-minimum/raii-assign-new.txt");
```

测试流程：

1. 检查两个文件打开成功。
2. target 写 old\n，source 写 before\n，每次检查写入结果。
3. 执行 target = std::move(source)。
4. 输出 target.valid() 和 source.valid()，预期 1、0。
5. target 写 after\n，并检查写入结果。
6. 补齐函数声明与 main 调用。

预期旧文件内容 old\n，新文件内容 before\nafter\n。
旧文件在移动赋值时关闭，新文件由 target 在析构时关闭。
这些是下一步的验收要求，当前尚未实现或运行验证。
