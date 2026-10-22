# F01 C 最小集（二）：数组、回调与任务链表

日期：2026-10-21
状态：进行中；单项练习已覆盖大部分主干，综合验收尚未完成。

接续：[第一部分](F01-c-minimum-part1.md)；课程状态见 [PROGRESS.md](../PROGRESS.md)。

更新说明（2026-10-22）：本文保留 2026-10-21 的学习状态。原计划的 push_task 已以 push_stack 命名实现，run_tasks 与混合回调实验也已完成，F01 已通过本轮验收。最新记录及下一步见 [F01 C 最小集（三）](F01-c-minimum-part3.md)和[课程进度](../PROGRESS.md)。

## 本次目标与当前边界

把指针、生命周期和回调组合成可创建、执行、清理的任务链表，为 libuv 的回调和 `.data` 建立语言基础。
当前实现的是任务链表，尚未完成完整的 FIFO 入队、出队与综合任务队列验收。
“F01 约八成”是学习范围的粗略估计，不是成绩或闸门通过结论。

## 1. 数组对象没有在传参时变成另一个对象

```c
Node* check_list[] = {node1, node2, node3};
```

- 在调用者中，`sizeof(check_list)` 计算整个数组；当前 x86-64 环境为 24 字节。
- 数组表达式在大多数使用位置会转换为首元素指针；`sizeof` 和一元 `&` 是常见例外。
- `Node* nodes[]` 作为函数形参会调整为 `Node** nodes`，函数收到地址的副本。
- 因而函数中的 `sizeof(nodes)` 是指针大小，当前环境为 8，不能据此求原数组长度。
- 原数组对象没有改变；长度应单独传入，例如 `print_nodes(check_list, count)`。

这包含两条规则：调用处的数组表达式转换，以及数组形式的形参声明调整，不能混成“只有传参才退化”。

## 2. 字符串与字节缓冲区

```c
char text[] = "abc";                 // 4 字节：a、b、c、\0
char* p = text;
char raw[] = {'a', 'b', 'c', 'd', 'e'}; // 5 字节，没有字符串结束符
```

- `p` 是指针，`text` 是数组；`p[3]` 是 `\0`，其数值为 0。
- C 字符串以第一个 `\0` 结束；`sizeof` 计算对象字节数，不扫描结束符。
- `%s` 需要可访问且以 `\0` 结束的字符序列，不能把任意缓冲区直接当字符串。
- `fwrite(raw, 1, sizeof raw, stdout)` 按明确长度写出 `abcde`，不依赖结束符。
- 换行是单独的输出，例如 `putchar('\n')`；它不属于这 5 字节缓冲区。

## 3. 函数指针、回调与上下文

```c
typedef int (*Operation)(int, int, void*);
```

`Operation` 是函数指针类型的别名。保存函数地址不会调用函数，执行 `operation(a, b, context)` 才调用。
先用 `add`、`subtract` 和 `calculate` 区分“决定做什么”与“决定何时调用”，再增加上下文。

`add_with_bias` 使用 `*(int*)context` 恢复具体类型，计算 `a + b + bias`。
`calculate_with_context(2, 4, add_with_bias, &bias)` 在 `bias == 10` 时得到 16。
`void*` 不记录实际类型，也不会延长对象生命周期；转换必须与真实对象类型一致。

同步调用期间可借用仍存活的局部变量；若局部变量所属函数已经返回，保存的地址就不能再解引用。
保存地址、复制指针或改用 `void*` 都不能修复生命周期结束的问题。

## 4. 本次选择的 Task / TaskNode 所有权

保留 `create_task`，并让 `TaskNode` 持有 `Task*`，没有改成节点内嵌 `Task`。

```text
TaskNode（堆） -> Task（堆） -> context / int bias（堆）
```

一个完整节点有三块独立堆内存。`struct TaskNode` 标签与 `TaskNode` 别名必须关联，才能让自引用 `next` 指向同一种节点。

- `prepare_task`：分配并初始化 `int` 上下文，设置回调；失败不发布未完成的上下文。
- `create_task`：分配 `Task`；准备失败时释放 `Task`，成功时交给调用者。
- `create_task_node`：分配节点；创建 `Task` 失败时释放节点，成功设置 `task` 和 `next = NULL`。
- 回调只借用 `context`；执行结束后，拥有任务的一方负责清理。

本轮固定使用 `TaskStatus`：`TASK_OK = 1`、`TASK_ERROR = -1`。
返回值与调用方判断保持一致；不能机械套用其他库的“0 成功”约定。

## 5. 分层清理、二级指针与别名

`destroy_task` 释放上下文并将 `context`、`operation` 置空，不释放 `Task` 对象本身。
`destroy_task_node` 依次清理上下文、释放 `Task`、释放节点，再将调用者的节点指针置空。

```c
TaskNode* next = current->next; // 必须在释放 current 之前保存
destroy_task_node(&current);
current = next;
```

链表遍历完成后还需要 `*head = NULL`；置空局部变量 `current` 不会自动置空调用者的 `head`。
`free(*node)` 释放堆上的节点，二级指针仍指向调用者的指针变量，所以随后可以写 `*node = NULL`。
置空一个指针不会同步修改其他别名；那些别名仍可能悬空，不能再访问已释放对象。
单节点清理和链表清理均支持重复调用、空对象及约定的 `NULL` 参数。

## 6. 链表顺序与单个任务失败策略

本次保留头插法。每次执行 `node->next = head; head = node;`，新节点都会成为链表头。
输入 bias 顺序为 `30、20、10` 时，遍历顺序为 `10、20、30`，回调参数 `2、4` 的结果为 `16、26、36`。
遍历每轮必须执行 `current = current->next`，否则会重复执行头节点，无法到达清理代码。

当前用户选择：单个节点创建失败时打印提示并跳过，保留已成功创建的任务，最后统一清理。
这与“失败立即全量回滚并退出”是不同契约；不强迫改尾插，也不把跳过策略本身判为泄漏。
分配函数内部仍必须释放本次失败途中已分配的资源。

## 7. 头文件、翻译单元与链接

`task.h` 保存 `Operation`、`TaskStatus`、`Task`、`TaskNode` 和函数声明。
`task.c` 包含自己的头文件及所用标准头文件，保存实现；`main.c` 包含头文件并调用接口。
CMake 将 `src/main.c` 和 `src/task.c` 加入同一目标，分别编译后再链接。

包含保护使用 `#ifndef F01_TASK_H`、`#define F01_TASK_H`、`#endif`。
保护在每个翻译单元自己的预处理过程中生效，不阻止不同 `.c` 分别包含同一个头文件。
严格语法检查已验证：没有先行包含其他头文件，连续包含 `task.h` 两次仍通过，类型只展开一次。

## 8. 宏、条件编译、static 与 ABI

- 宏是预处理替换；表达式宏的参数及整体应加括号，避免运算符优先级改变原意。
- `#if TASK_TRACE` 按宏值决定是否保留 trace 代码；`#ifdef TASK_TRACE` 只判断是否定义，即使定义为 `0` 也成立。
- `enum` 给成功、失败这些状态命名；本次 `TaskStatus` 已使用明确的 `1/-1` 值。
- 局部 `static` 变量具有静态存储期，函数返回后仍存活，但名字的作用域仍局限于所在代码块。
- 文件作用域的 `static` 函数具有内部链接，其他翻译单元不能直接按该名字链接它。
- 内部链接不妨碍把它的函数地址作为回调传出去；接收方通过函数指针调用。
- 不同翻译单元若对同一结构体采用不一致的成员布局，访问成员时会使用不同偏移。
- 即使 `sizeof` 碰巧相同，也不能证明布局一致；链接器通常不会替我们核对这些 C 类型契约。

static 生命周期、函数指针可传递性及 ABI 成员偏移问题已完成口头检查。
源码对照：[libuv v1.52.1 的 test/test-timer.c](https://github.com/libuv/libuv/blob/v1.52.1/test/test-timer.c)；结合定时器回调理解函数地址的登记与后续调用。

## 9. 已取得的验证证据

严格编译参数：`-std=c11 -Wall -Wextra -Wpedantic -Werror`。
正常工程构建和运行通过；本轮亦使用过 `/tmp/f01-rebuild.9krDHR` 的独立构建目录。

```bash
cd /root/workspace/node-runtime-lab
source scripts/activate-lab.sh
cmake -S labs/00-foundations/c-minimum -B /tmp/f01-rebuild.9krDHR -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/f01-rebuild.9krDHR --clean-first --target f01-pointers -j2
/tmp/f01-rebuild.9krDHR/f01-pointers
```

| 隔离检查 | 实际结果 |
| --- | --- |
| 创建节点的第 1/2/3 次 malloc 分别失败 | 成功分配部分全部释放，剩余分配均为 0 |
| 单个节点创建后销毁 | 3 次分配、3 次释放，调用者节点指针为 NULL |
| 三节点链表执行与清理 | 回调 16/26/36；9 次分配、9 次释放，head 为 NULL |
| 再次清理、空链表、NULL 参数 | 不新增释放，无无效或重复 free，无崩溃 |

故障注入与分配跟踪来自 `/tmp` 中引用真实源码的隔离驱动，不是 `main` 内新增的故障注入。
相关驱动：`/tmp/f01-task-node-pointer-fail-gpllpjmb/harness.c`、`/tmp/f01-destroy-task-node-qmir8n4q/harness.c`、`/tmp/f01-destroy-task-list-jblo4xnn/harness.c`。
ASan/UBSan 链接所需库缺失，没有完成 sanitizer 运行；不能将上述计数验证写成完整的消毒器验证。

## 10. 下一步：尚未实施的 push_task 与综合验收

刚布置的接口为 `TaskStatus push_task(TaskNode** head, Operation operation, int bias)`：

1. `head == NULL` 或 `operation == NULL` 返回 `TASK_ERROR`；`*head == NULL` 是合法空链表。
2. 复用 `create_task_node(bias)`；失败返回 `TASK_ERROR`，原链表保持不变。
3. 成功后设置 `node->task->operation = operation`，头插到 `*head`，返回 `TASK_OK`。

该接口尚未实现和验收。之后仍需完成带回调与上下文的小型任务队列综合实验、失败路径检查和脱稿解释；F01 继续标记为进行中。
