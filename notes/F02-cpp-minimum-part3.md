# F02 C++ 最小集（三）：移动赋值、模板、多态与 lambda

日期：2026-10-29
学习范围：2026-10-28 至 2026-10-29。
状态：本轮列出的课堂练习已通过，F02 整体仍在进行中。

接续：[第一部分](F02-cpp-minimum-part1.md)、[第二部分](F02-cpp-minimum-part2.md)；总进度见 [PROGRESS.md](../PROGRESS.md)。
代码：[main.cc](../labs/00-foundations/cpp-minimum/src/main.cc)、[file_raii.cc](../labs/00-foundations/cpp-minimum/src/file_raii.cc)。

本次下班提交按用户要求不重新检查代码或运行构建；下述验证引用此前课堂已取得的构建与运行结果，不是本次再次验收。

## 1. File 移动赋值：分清两层对象

本轮完成 `File& operator=(File&& other) noexcept`：处理自移动、关闭目标旧流、接管源流、置空源并返回 `*this`。
自移动 guard 已进行源码审查，没有单独执行自移动测试，不能写成该分支已实测通过。

实验中 target/source 是两个 `unique_ptr<File>`，实际正确调用为：

```cpp
*target = std::move(*source);
```

这调用两个已经存在的 File 之间的移动赋值。两个智能指针仍非空，源 File 内部流为空，目标 File 接管资源。
此前误写的 `target = std::move(source)` 调用的是 unique_ptr 移动赋值，会使 source 智能指针为空；随后通过它解引用不合法。
移动构造创建新对象，移动赋值处理已有对象；直接覆盖目标旧句柄而不清理，会丢失原资源的关闭责任。

此前七个练习文件已核对内容；前五项也见第二部分。本次不重新读取文件：

| 场景 | 已验证内容 | 字节数 |
| --- | --- | --- |
| 正常退出 | `normal\n` | 7 |
| 提前 return | `early\n` | 6 |
| 引用借用后继续写 | `borrowed\nowner\n` | 15 |
| 移动构造 | `moved\n` | 6 |
| 按值接管 | `owned\n` | 6 |
| 移动赋值：目标原文件 | `old\n` | 4 |
| 移动赋值：源文件被接管后继续写 | `before\nafter\n` | 13 |

## 2. shared_ptr：复制所有者，不复制 Trace

使用已有 Trace，外层 `p = std::make_shared<Trace>()`，内层 `q = p`，没有 std::move。
观察到 p 初始计数 1，复制给 q 后计数 2。
执行 p.reset() 后，p 为空的检查为 1，p.use_count() 为 0；q 仍持有对象，计数为 1。
内层结束时最后一个所有者 q 释放，才出现 Trace 的 destroy，随后输出 after inner scope。

reset 只放弃当前这一份所有权，不保证当场销毁共享对象；本例只有一个 Trace，没有复制出第二个 Trace。
本轮只观察单线程中的共享所有权，没有展开线程、循环引用或 weak_ptr。

## 3. 函数模板与类模板

```cpp
template <typename T>
T larger(T a, T b) {
  return a > b ? a : b;
}
```

实际调用 larger<int>(3,7)、larger<double>(2.5,1.2)、larger(8,5)，输出为 7、2.5、8。
前两次显式指定模板参数，第三次由实参推导 T；本例两个参数使用同一个 T。

ValueBox 已改为成员初始化列表：

```cpp
template <typename T>
class ValueBox {
 public:
  explicit ValueBox(T value) : value_(value) {}
  T get() const { return value_; }
  void set(T value) { value_ = value; }
 private:
  T value_;
};
```

ValueBox<int> 和 ValueBox<float> 的初始读取、修改后读取输出为 10、2.5、20。
初始化列表在进入构造函数体前初始化成员；在函数体中赋值是另一件事，内置类型成员不会因此自动先成为 0。
初始化列表可初始化 const 成员，但不能把当前成员改成 const 后仍保留修改它的 set。

命名空间部分用户表示已熟悉，未再单独完成一组命名空间代码实验；已纠正不能随意向 std 添加自定义内容。
限定名用于说明名字所属作用域，不创建对象，也不改变模板计算结果。

## 4. 继承、虚函数与抽象类

从基类默认加法与派生类乘法开始，再把 Operation::run 改成纯虚函数：

```cpp
virtual int run(int a, int b) const = 0;
```

AddOperation 与 MultiplyOperation 都使用签名匹配的 const override，calculate 接受 const Operation&。
输入 (2,4) 实际输出为 6、8，基类引用能够调用实际派生对象的覆写。
原来的 Operation& 也是合法接口；增加 const 表达只读借用，并允许 const 对象，不是动态分派的开关。

没有 virtual 时，通过基类引用调用这类成员按静态类型选择；有 virtual 才发生相应动态分派。
派生到基类的引用转换不复制对象，不会切片；对可实例化基类按值复制派生对象则可能切片。
override 让编译器核对覆写签名，不能遗漏本例中的 const。

=0 是纯虚声明标记，不是返回数值 0。Operation 不能直接实例化，但可以作为引用或指针类型。
本例没有为纯虚 run 提供默认函数体；不能由此推论纯虚函数在任何情况下都不允许另有类外定义。

## 5. 基类智能指针与虚析构

内层作用域使用 unique_ptr<Operation> 接管 make_unique<MultiplyOperation>()，调用 calculate(*op,3,4)。
抽象基类指针可以拥有实际派生对象；转换转交所有权，不复制或切片该对象。
基类虚析构保证经基类类型销毁时先执行派生析构，再执行基类析构；派生析构即使省略 override 仍继承虚性质。

该新增测试片段的实际输出是：

```text
test_polymorphic_owner
12
MultiplyOperation destroy
Operation destroy
after owner scope
```

旧 test_polymorphism 中的局部对象也新增析构日志，不能把其他测试的日志混成这段片段的输出。

## 6. lambda：无捕获与两种捕获

test_lambda 已声明并在 main 调用；没有强制另外创建 test_capture，捕获练习保留在同一函数末尾。
无捕获 add 和 multiply 使用 (4,5)，实际输出 9、20，按实际参数验收，不沿用最初布置的 6、8。
auto 推导 lambda 对象类型，[] 是捕获列表，参数和函数体描述调用行为，声明末尾仍需要分号。

随后 num=10 时创建 by_ref=[&num] 与 by_value=[num]，两者均返回 a+b+num。
再令 num=20，先调用 by_ref(4,5)，后调用 by_value(4,5)，实际新增输出为：

```text
29
19
```

引用捕获读取仍存活的原变量，值捕获保留创建时复制的 10；引用捕获不延长变量生命周期。
本轮没有引入 mutable 或 std::function，也没有把带捕获 lambda 当作普通函数指针。

## 7. 下一次明确从这里继续

刚布置但尚未保存、尚未验证：将已有无捕获 lambda 转成签名匹配的函数指针，再通过指针调用。
下一次先完成这一小步，再进入 C 回调的上下文指针桥接，不把已布置当成已完成。

F02 后续仍待：lambda 与 C 回调桥接、V8 Maybe/MaybeLocal/Local<T> 类型阅读、故意 double-free/use-after-free 的定位修复实验，以及综合验收。
本轮未新做 sanitizer 或分配失败注入；不能把普通构建运行和既有文件检查扩写成这些验证。
覆盖这些小主题不等于已熟练掌握完整 C++，当前继续按最小集推进。


## 后续进展（2026-10-30）

无捕获 lambda 转函数指针、显式 context 与捕获对象桥接现已通过；V8 包装类型练习和资源错误诊断见 [第四部分](F02-cpp-minimum-part4.md)。本篇保留截至 2026-10-29 的历史状态；下次从尚未保存的 Buffer double-free 修复继续，F02 仍在进行中。
