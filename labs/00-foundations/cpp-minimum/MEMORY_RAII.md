# F02：故意制造并修复资源生命周期错误

日期：2026-10-30。源码：[src/memory_raii.cc](src/memory_raii.cc)。

这是独立的资源生命周期教材。下文保留最初的 double-free、use-after-free 错误过程，
但**当前源码的两项错误均已修复**，修复版分别通过 Valgrind：ERROR SUMMARY 为 0、无泄漏、退出码为 0。
不传参数时仍只显示用法，不执行实验。F02 整体仍在进行中，下一步为短综合验收。

## 构建与观察

以下命令用于运行当前修复版；本次文档更新没有重新构建或运行程序，结果引用此前课堂验证。

```bash
cd /root/workspace/node-runtime-lab &&
source scripts/activate-lab.sh &&
cmake -S labs/00-foundations/cpp-minimum -B build/f02-cpp-minimum -DCMAKE_BUILD_TYPE=Debug &&
cmake --build build/f02-cpp-minimum --target f02-memory-raii &&
valgrind --tool=memcheck --leak-check=full --track-origins=yes --error-exitcode=99 ./build/f02-cpp-minimum/f02-memory-raii double-free
```

另一个模式将最后的程序参数改为 `use-after-free`。
VS Code：F1 → Tasks: Run Task → F02: diagnose double free 或 F02: diagnose use after free。
任务名称和模式名称保留原故障名称，不代表当前源码仍有该错误。
发现错误时 Valgrind 按 --error-exitcode=99 返回 99；当前两个修复版都已确认返回 0。
使用现有 Valgrind 3.26.0；ASan/UBSan 运行库缺失，没有运行它们，也没有新增安装。

## 第一项：重复释放，原错误已复现

Buffer 使用 malloc 分配一个 int，保存 42，析构调用 free。
原先默认复制只复制 data_ 的地址，不重新分配资源：

```cpp
Buffer first;
{
  Buffer second = first;  // 原错误示例，当前源码已改成引用
}
```

内层结束时 second 释放一次，外层结束时 first 对同一地址再次释放。
课堂诊断中的地址示例为 0x4da4c80；地址不保证每次相同。

错误版的实际诊断包括：

```text
Invalid free() / delete / delete[] / realloc()
All heap blocks were freed -- no leaks are possible
ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 0 from 0)
```

Valgrind 分别定位到析构中的 free、内层结束的首次释放、外层结束的再次释放和最初的 malloc。
错误版通过子进程确认退出 99；无泄漏与存在重复释放可以同时成立。
原诊断记录：[valgrind-double-free-before-fix.log](../../../build/f02-cpp-minimum/valgrind-double-free-before-fix.log)。

### 修复结果：禁止复制，改为引用借用

当前类的复制构造和复制赋值均为 = delete：

```cpp
Buffer(const Buffer&) = delete;
Buffer& operator=(const Buffer&) = delete;
```

测试内层现在使用 `Buffer& second = first`，读取前检查分配成功。
second 是 first 的别名，没有创建第二个资源所有者；内层结束不会析构另一个 Buffer。
first 在函数结束时才析构，唯一一次释放发生在 after inner scope 之后。
源码保留的 copy value 输出文案是历史标签，不表示当前还在复制对象。

该修复已在上一轮课堂通过：仅释放一次，ERROR SUMMARY 为 0、无泄漏、退出码为 0。
修复日志：[valgrind-double-free-after-fix.log](../../../build/f02-cpp-minimum/valgrind-double-free-after-fix.log)。

## 第二项：释放后访问，原错误与修复

原代码把 owner.data() 的借用地址保存到外部，在 owner 离开内层作用域后读取：

```cpp
int* saved = nullptr;
{
  Buffer owner;
  // 分配成功后：
  saved = owner.data();
}
std::cout << *saved;  // 原错误位置：owner 已经析构
```

原模式已通过 Valgrind 运行，记录见
[valgrind-use-after-free-before-fix.log](../../../build/f02-cpp-minimum/valgrind-use-after-free-before-fix.log)。
saved 非空只说明指针值不是 nullptr，不能证明所指对象仍存活；碰巧读到 42 也不表示访问合法。

### 修复结果：所有者存活到最后一次读取

当前将 `Buffer owner` 的声明放到函数外层，与 saved 同在函数作用域。
内层只检查 owner.valid() 并保存 owner.data()；分配失败会提前返回，成功后才在块外读取。
块外读取发生在 owner 析构之前，最后仍由 owner 析构释放内存，没有删除读取或泄漏资源来掩盖错误。

课堂实际读取与释放顺序为：

```text
borrowed value: 42
free 0x4da4c80
```

地址仅为该次运行示例。关键是先完成最后一次读取，再释放。
本轮 Valgrind 已确认 ERROR SUMMARY 为 0、无泄漏、退出码为 0。
修复日志：[valgrind-use-after-free-after-fix.log](../../../build/f02-cpp-minimum/valgrind-use-after-free-after-fix.log)。

## 证据范围与下一步

上述日志位于本机被 Git 忽略的 build/f02-cpp-minimum，不随课程源码提交。
这两项检查覆盖本次 double-free 与 UAF 修复路径，不等同于所有输入和所有资源错误均已验证。
没有运行 ASan/UBSan 或分配失败注入，也没有为通过检查而删除析构释放。

下一步使用已有代码做短综合验收：说明谁拥有资源、谁只是借用、对象何时析构，
并解释回调 context 的生命周期以及 Maybe/MaybeLocal 的失败输出约定。
两项资源错误修复已经完成，F02 整体验收仍待完成。
