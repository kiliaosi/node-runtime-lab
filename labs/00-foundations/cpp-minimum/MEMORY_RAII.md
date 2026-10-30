# F02：故意制造并修复资源生命周期错误

日期：2026-10-30。源码：[src/memory_raii.cc](src/memory_raii.cc)。

这是独立的故障教材，当前代码故意保留 double-free 和 use-after-free。
截至本次提交，仅 double-free 错误版已复现；两个案例的修复均未完成。
不传参数时只显示用法，不执行故障。

## 构建与观察

```bash
cd /root/workspace/node-runtime-lab &&
source scripts/activate-lab.sh &&
cmake -S labs/00-foundations/cpp-minimum -B build/f02-cpp-minimum -DCMAKE_BUILD_TYPE=Debug &&
cmake --build build/f02-cpp-minimum --target f02-memory-raii &&
valgrind --tool=memcheck --leak-check=full --track-origins=yes --error-exitcode=99 ./build/f02-cpp-minimum/f02-memory-raii double-free
```

VS Code：F1 → Tasks: Run Task → F02: diagnose double free。
另有 F02: diagnose use after free，对应程序参数 use-after-free，留待下一项实验。
发现错误时 Valgrind 按 --error-exitcode=99 返回 99；修复后应返回 0。
当前使用已安装的 Valgrind 3.26.0；ASan/UBSan 运行库缺失，未运行它们。

## 第一项：重复释放，已复现但待修复

Buffer 使用 malloc 分配一个 int，保存 42，析构调用 free。
默认复制只复制 data_ 的地址，不重新分配资源：

```cpp
Buffer first;
{
  Buffer second = first;
}
```

内层结束时 second 释放一次，外层结束时 first 对同一地址再次释放。
课堂诊断中的地址示例为 0x4da4c80；地址不保证每次相同。

实际诊断包括：

```text
Invalid free() / delete / delete[] / realloc()
All heap blocks were freed -- no leaks are possible
ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 0 from 0)
```

Valgrind 分别定位到析构中的 free、内层结束的首次释放、外层结束的再次释放和最初的 malloc。
课堂通过子进程确认 Valgrind 返回 99；无泄漏与存在重复释放可以同时成立。
本机课堂日志保存在被 Git 忽略的 build/f02-cpp-minimum/valgrind-double-free-before-fix.log。

### 当前学生任务

1. 将复制构造和复制赋值的 = default 改为 = delete。
2. 将内层 Buffer second = first 改为 Buffer& second = first，表达借用。
3. 再运行 double-free 任务，确认只有一次 free、ERROR SUMMARY 为 0、退出码为 0。

这三项修复尚未验证，不能把当前故障教材记为安全版本。

## 第二项：释放后访问，待运行和修复

test_use_after_free 把 owner.data() 的借用地址保存到外部。
owner 离开作用域后已经释放内存，随后解引用 saved 是 use-after-free。
下一步通过 Valgrind 观察 Invalid read 及分配/释放位置，再调整所有者与借用的生命周期。
当前只完成代码准备，尚未运行此模式，也未修复。

完成两项实验后再进行 F02 综合验收。
