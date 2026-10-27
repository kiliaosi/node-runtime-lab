# F02 C++ 最小集（一）：引用、对象生命周期与独占所有权

日期：2026-10-22
状态：引用部分的口头检查和四个生命周期/所有权代码实验已完成；F02 整体仍在进行中。

前序：[F01 综合验收](F01-c-minimum-part3.md)；总进度见 [PROGRESS.md](../PROGRESS.md)。
代码见 [main.cc](../labs/00-foundations/cpp-minimum/src/main.cc)，复现说明见 [实验 README](../labs/00-foundations/cpp-minimum/README.md)。

## 1. 本轮范围与证据

- 口头练习：值复制与引用、引用不能重新绑定、引用形参、指针的引用、const 引用、返回局部变量引用的悬空问题。
- 手写实验：Trace、test_scope、test_unique、test_move、custom_trace 和 test_transfer；main 已调用四组实验。
- 验证：Rocky Linux 上 GNU C++ 14.2.1、C++20、-Wall -Wextra -Wpedantic -Werror，重新构建与运行通过，没有警告，进程退出 0。
- 本轮验证依据是实际源码、严格构建和输出顺序；没有运行 ASan/UBSan，也没有做分配失败注入。
- 本轮通过的是对象生命周期与 unique_ptr 所有权这一组练习，不表示 F02 全部完成或已经独立熟练。

## 2. 引用：给已有对象另一个名字

```cpp
int n = 10;
int copy = n;
int& ref = n;
copy = 20;
ref = 30;
// n == 30，copy == 20，ref == 30
```

copy 是独立对象；ref 是 n 的别名。引用初始化后不能重新绑定。
如果 ref 绑定 a，执行 ref = b 是把 b 的值赋给 a，不是把 ref 改绑到 b。

| 练习 | 已确认的结论 |
| --- | --- |
| void by_value(int x) | 修改 x 不改变调用者的 int 对象 |
| void by_reference(int& x) | 形参 x 绑定调用者对象，通过 x 修改会影响它 |
| void clear_pointer(int*& p) 中 p = nullptr | 修改调用者的指针变量；其原来指向的 int 对象不会因此变成 0 或被销毁 |
| const int& ref = n，随后 n = 20 | ref 读到 20；不能通过 ref 给 n 赋值，但 n 仍可经自身修改 |
| 返回指向函数内普通局部 int 的引用 | 返回后局部对象已销毁，引用悬空，不能据此访问对象 |

引用不会让这个普通局部变量在函数返回后继续存活。这与 C 中返回局部变量地址的问题相通。
以上是口头练习结论，未把它们描述成仓库内已有的自动化测试。

## 3. 构造、析构与 RAII

Trace 构造时输出 create，析构时输出 destroy。构造函数和析构函数没有返回类型，结构体定义以分号结束。

test_scope 在内层花括号内创建局部 Trace。离开该块时自动析构，输出顺序是：
A -> create -> B -> destroy -> C。

Trace 只打印日志，用于观察对象生命周期；它本身没有封装文件等额外资源。
RAII 的核心是把资源的管理责任绑定到对象生命周期，由析构完成清理。

test_unique 中有两个不同对象：

```cpp
std::unique_ptr<Trace> p = std::make_unique<Trace>();
```

- p 是局部智能指针对象。
- make_unique 动态创建 Trace，并把所有权交给智能指针。
- p 析构时，若仍然拥有 Trace，就销毁 Trace 并释放对应内存。
- 不手动 free 或 delete 它正在管理的对象。

最初两个实验都输出 A/create/B/destroy/C。随后在 test_unique 打印 B 后加入 return；
当前保存的是这个提前返回版本，所以它输出 A/create/B/destroy，不输出 C。
学生先正确预测了该结果，再通过编译运行验证。

正常走到作用域末尾或提前 return，都能触发相应局部对象的析构，减少每个返回分支重复写资源清理的需要。
这个结论没有通过本轮实验扩展到直接终止进程等其他退出方式。

## 4. unique_ptr 的空状态与所有权移动

```cpp
std::unique_ptr<Trace> p;
```

这种默认构造只得到空智能指针，不会创建 Trace。最初误用了这个写法，已改成 make_unique。

test_move 在外层创建 p，在内层执行：

```cpp
std::unique_ptr<Trace> q = std::move(p);
```

std::move 本身允许把表达式用于移动；这里实际接管资源的是 unique_ptr 的移动构造函数。
执行后 p 为空，q 管理原来那个 Trace；Trace 没有重新创建或被复制，地址也没有因为这次转交而改变。
不能通过复制 unique_ptr 让两个所有者分别负责释放同一对象。

内层输出 p empty: 1、q empty: 0。内层结束时 q 析构并销毁 Trace，随后才输出 after inner scope。
外层 p 仍是存在的变量，但已为空；它最终析构时不会再次销毁那个 Trace。
此处“移动后为空”是 unique_ptr 的语义，不推广为所有 C++ 类型移动后的统一状态。

## 5. 把所有权交给函数

实际函数名称保留学生选择的 custom_trace，布置练习时曾称为 consume_trace。

```cpp
void custom_trace(std::unique_ptr<Trace> item);
```

test_transfer 中创建调用者的 item，再执行：

```cpp
custom_trace(std::move(item));
```

两个函数里的 item 是两个不同的智能指针对象。参数按值构造，通过移动接管所有权；
没有复制 Trace，也没有重新创建一个 Trace。

custom_trace 内打印参数 item empty: 0。当前函数没有把所有权继续转交出去，调用完成时 Trace 已销毁。
随后调用者打印 after call 和 caller item empty: 1。
调用者的智能指针变量仍然存在，所持资源已经转交；这是所有权转移，不是借用。

## 6. 最终程序的实际输出

下面对应当前 main.cc，包含 test_unique 中的提前 return：

```text
scope
A
create
B
destroy
C
unique
A
create
B
destroy
move
create
p empty: 1
q empty: 0
destroy
after inner scope
transfer
create
before call
inside custom_trace
item empty: 0
destroy
after call
caller item empty: 1
```

每组实验各创建一个 Trace，各销毁一次。调用顺序为：
main 依次调用 test_scope、test_unique、test_move 和 test_transfer；
最后一组内部由 test_transfer 调用 custom_trace。

## 7. 本轮纠正与自查习惯

- struct Trace 末尾需要分号；std::puts 的命名空间不能误写为 str。
- void 函数提前返回写 return;，不能写 return 0;。
- 默认构造空 unique_ptr 与 make_unique 创建对象是不同操作。
- 使用 std::unique_ptr<Trace> 的函数声明应在类型已声明之后；当前把声明放在完整 Trace 定义之后、main 之前。
- 函数定义可以放在 main 后面，但调用前要有可见声明。
- 函数写好但 main 没调用时，编译成功也不能证明实验已经执行。
- 空函数未使用参数，在当前 -Werror 选项下会导致构建失败；补齐实验需要的观察输出即可使用该参数。
- 保存后核对实际编译文件。当前源文件是 src/main.cc，CMake 已同步，不再是最初脚手架中的 main.cpp。
- 每次修改同时检查声明、实现、main 调用和观察输出，减少分多轮补漏。

## 8. 与 Node.js 源码的联系

课堂对照过官方 v24.20.0 源码：[TCPWrap](https://github.com/nodejs/node/blob/v24.20.0/src/tcp_wrap.cc) 的 New 中有 new TCPWrap；
[environment.cc](https://github.com/nodejs/node/blob/v24.20.0/src/api/environment.cc) 的 ArrayBufferAllocator::Create 使用 make_unique。
调用处没有直接出现 new，不代表没有动态对象；仍要继续追踪创建、所有权和销毁路径。
这是源码阅读对照，没有在本轮编译或运行 Node.js。

## 9. 下一次从这里继续

下一步亲手写一个管理真实 C 资源的小型 RAII 类，把成员、构造/析构、资源有效性和失败路径串起来，
并明确复制行为，避免默认复制一个资源句柄后重复释放。本次尚未开始实现该类。

后续还包括 shared_ptr 的所有权差异、模板与命名空间、继承/虚函数的阅读能力、
lambda 与 C 回调桥接，以及 V8 的 Maybe/MaybeLocal/Local 等包装类型；按课程顺序继续，不在本次标记完成。
本轮只练习了 unique_ptr 提供的移动行为，尚未手写资源类的移动构造或移动赋值。

后续采用完整小任务：一次说明声明、实现、调用和验证点，保留关键生命周期检查，减少已掌握输出题的重复确认。

## 后续进展（2026-10-27）

随后已完成六组真实 File 资源实验，包括失败处理、引用借用、移动构造和按值参数接管。
最新验证及下一次的移动赋值任务见 [第二部分](F02-cpp-minimum-part2.md)；以上章节保留 2026-10-22 的学习记录。
