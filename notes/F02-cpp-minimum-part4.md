# F02 C++ 最小集（四）：回调桥接、V8 包装类型与资源错误诊断

日期：2026-10-30
状态：本轮回调桥接和包装类型练习已验证；资源错误教材已观察，修复尚未完成；F02 仍在进行中。

前序：[第三部分](F02-cpp-minimum-part3.md)；历史基础见 [第一部分](F02-cpp-minimum-part1.md)、[第二部分](F02-cpp-minimum-part2.md)；总进度见 [PROGRESS.md](../PROGRESS.md)。
代码：[main.cc](../labs/00-foundations/cpp-minimum/src/main.cc)、[v8_maybe.cc](../labs/00-foundations/cpp-minimum/src/v8_maybe.cc)、[memory_raii.cc](../labs/00-foundations/cpp-minimum/src/memory_raii.cc)。

今天提交不重复编译或运行，以下引用此前课堂已取得的实际结果；没有把未保存的修复写成已通过。

## 1. 无捕获 lambda 转函数指针

test_lambda 末尾复用已有无捕获 add：

```cpp
int (*callback)(int, int) = add;
std::cout << callback(2, 4) << std::endl;
```

该段实际输出依次为 `9 / 20 / 29 / 19 / 6`，最后的 6 来自新函数指针调用。
无捕获 lambda 可转换为签名匹配的函数指针；前面带 num 捕获的两个 lambda 不支持这种直接转换。
函数指针只说明如何调用，不会自动保存捕获对象或延长其生命周期。

## 2. 显式 context：同步借用 int

```cpp
typedef int (*ContextCallback)(int, int, void*);
int invoke_callback(ContextCallback cb, void* context, int a, int b) {
  return cb(a, b, context);
}
```

无捕获 callback 在函数体中将 context 用 static_cast<int*> 还原，再计算 a+b+*value。
局部 bias 初始为 10，调用 invoke_callback(callback,&bias,2,4) 输出 16；修改 bias 为 20 后再次调用，输出 26。
转换恢复真实对象类型，不分配 int，也不转移所有权。
这里是同步 C 风格接口模拟，没有声称本轮已经接入某个真实 C 库。

## 3. 捕获对象通过 void* 桥接

桥接代码追加在同一个 test_context_callback 末尾，没有另外实现 test_lambda_bridge。
此时 bias 已经是 20，再创建值捕获对象 work：

```cpp
auto work = [bias](int a, int b) { return a + b + bias; };
typedef decltype(work) WorkType;
ContextCallback bridge = [](int a, int b, void* context) {
  WorkType* local_work = static_cast<WorkType*>(context);
  return (*local_work)(a, b);
};
std::cout << invoke_callback(bridge, &work, 2, 4) << std::endl;
```

实际新增输出为 26。最初示例中的 16 对应在 bias=10 时创建 work，差异来自捕获时机。
decltype(work) 取得闭包类型，不执行或复制 work；typedef 只是给该类型命名。
work 是有捕获的对象，bridge 是无捕获 lambda 转成的函数指针，状态通过 &work 单独传入。
传入对象地址和还原类型必须对应；不能传 &bias 却把它解释成 WorkType*。
两次 int context 调用及闭包桥接都在相应局部对象仍存活时同步完成；context 不拥有这些对象。

## 4. 真正的 V8 Maybe<T> 头文件练习

独立 v8_maybe.cc 使用安装的 Node v24.20.0 头文件：
`/opt/node-v24.20.0-linux-x64/include/node`。
本轮基本 Maybe<int> 与默认空句柄练习为 header-only 使用方式，无需初始化 V8；不能把这个结论推广到创建 JS 对象。

value=Just<int>(42)、empty=Nothing<int>()，实际顺序为：

| 表达式 | 实际结果 |
| --- | --- |
| value.IsJust() | 1 |
| value.FromMaybe(-1) | 42 |
| empty.IsJust() | 0 |
| empty.FromMaybe(-1) | -1 |

empty.IsJust 附近旧注释仍误写“打印 1”，应以实际返回 0 为准；判断空状态为真可以使用 IsNothing()。
“有值”与数值非零不是同一回事，Just(0) 仍表示有值。
FromMaybe 给调用者备用结果，不把 Nothing 改成 Just；没有运行空值上的 FromJust。

## 5. Maybe::To：失败保留原 out

value.To(&out) 返回 true 并写入 42。学生连续两次调用 value.To，第二次仍将 out 从 -99 写回 42。
因此若立即调用 empty.To，会失败并保留 42；这不是 To 实现错误，而是调用前 out 的实际状态。
最后在 empty.To 前明确重设 out=-99，实际日志为：

```text
empty to: 0 -99
```

返回 false 表示没有写出值，旧 out 不能被当成 Maybe 内部成功返回的结果。
本轮已经通过实际运行观察成功写出和失败保留原值。

## 6. Local / MaybeLocal 与生命周期：分清实测和阅读

test_maybe_local 默认构造 Local<String> text、MaybeLocal<String> result，调用 result.ToLocal(&text)。
学生当前只打印失败返回值，实测为 0；没有增加真实 JS 对象创建或非空句柄运行实验。
默认 Local 和 MaybeLocal 可以为空；empty 表示没有可用句柄，不是 JS 的 null 或 undefined。

已阅读的 API 契约：MaybeLocal::ToLocal 失败时返回 false，并将输出 Local 设为空。
这与 Maybe<int>::To 失败保留原 out 不同；当前 out 本来为空，不能单凭这次运行证明旧非空 out 被清空。

以下生命周期认识来自代码片段和 API 阅读，而不是本轮引擎运行验证：
- Local 的有效期受创建句柄的 HandleScope 管理；不是任意一个 C++ 花括号结束都会使所有 Local 失效。
- 创建句柄的 HandleScope 结束后，不能继续使用那些失效的 Local；复制 C++ 句柄变量不能修复它。
- HandleScope 退出释放句柄，不意味着对应 JS 对象立即被 GC。
- Local 不是 unique_ptr 式的对象所有者，不能对它管理的 JS 对象执行 delete。
- Global / 强 persistent 引用可以保活对象；Reset 解除这份引用，但不保证对象立即回收。
- 弱句柄不等同于强引用；这里不把强引用的保活结论推广到弱句柄。

## 7. Valgrind 观察故意错误的 Buffer 教材

memory_raii.cc 是助手提供的故意错误参考材料，不是已验收 File 的新实现，也不声称学生已独立写好修复。
Buffer 用 malloc 管理 int，成功时写入 42，析构释放；默认浅拷贝让两个对象保存同一个地址。
内层 `Buffer second = first` 结束时先释放一次，外层 first 析构再次释放同一地址，形成 double-free。

Valgrind 3.26.0 课堂实测记录：
- 日志显示同一个地址被 free 两次，并报告 Invalid free。
- ERROR SUMMARY 为 1，使用 `--error-exitcode=99` 后退出码为 99。
- 没有遗留堆内存不等于没有内存错误；首次有效释放后仍可能发生非法重复释放。

程序必须显式选择故障模式，无参数只显示 usage，不默认执行故障。
UAF 模式已经写入教材，但尚未运行或修复，不能记成已复现 use-after-free。
ASan/UBSan 所需库缺失，本轮没有运行它们；使用现成 Valgrind，没有为此安装依赖。

## 8. 下一次从修复 double-free 继续

刚布置、尚未保存或验收的修复：
- 删除 Buffer 的复制构造和复制赋值，阻止复制出第二个资源所有者。
- 将本次 second 改为对 first 的引用借用，而不是另建一个负责释放的对象。
- 用实际输出和 Valgrind 重新检查只释放一次、没有错误；目前不能提前写成退出 0。

随后才执行并修复 UAF 模式，核对借用指针与所有者的生命周期，再进行 F02 综合验收。
本轮已覆盖 V8 包装类型入门阅读，不等于完成 V8 初始化、HandleScope 成功路径或完整 embedder。
