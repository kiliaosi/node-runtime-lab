# F01 C 最小集（三）：任务注册、跨模块回调与综合验收

日期：2026-10-22
状态：本轮 F01 验收通过；范围覆盖不等于已经熟练掌握所有边界。

前序：[第一部分](F01-c-minimum-part1.md)、[第二部分](F01-c-minimum-part2.md)；总进度见 [PROGRESS.md](../PROGRESS.md)。

## 1. 本轮完成了什么

在已有创建与清理基础上，完成任务注册接口、跨模块执行接口，以及别名、上下文和失败路径的口头检查。
实际接口命名是 `push_stack`，不是上次笔记中尚未实现的 `push_task`。

```c
TaskStatus push_stack(TaskNode** head, Operation operation, int bias);
void run_tasks(TaskNode* node, int a, int b);
```

保留用户选择：`create_task` 独立存在，`TaskNode` 持有 `Task*`，使用头插法。
它是按约定验收的回调任务链表，头插具有 LIFO 语义；没有实现或宣称完整 FIFO 队列。

## 2. push_stack：登记，不执行

- `head == NULL` 或 `operation == NULL` 返回 `TASK_ERROR`，不分配内存。
- `*head == NULL` 表示合法空表，不是参数错误。
- 调用 `create_task_node(bias)`；失败返回 `TASK_ERROR`，保留原表。
- 成功后设置 `node->task->operation = operation`，头插到 `*head`，返回 `TASK_OK`。
- 登记时只保存函数地址和上下文，不执行回调。

`TaskStatus` 延续 `TASK_OK = 1`、`TASK_ERROR = -1` 的约定，调用方按这个约定判断。
声明、定义和调用必须一致：链表头是 `TaskNode*`，修改调用者头指针需要 `TaskNode**`，不能写成 `Task**`。

## 3. main 中的不同回调与实际执行顺序

`main.c` 新增 `static multiply_with_bias`，计算 `a * b + bias`。

```c
int bias_array[] = {30, 20, 10};
Operation operations[] = {add_with_bias, multiply_with_bias, add_with_bias};
```

按索引配对登记，头插后先执行最后登记的任务。`main` 使用 `run_tasks(current, 2, 4)`，其中 `current = head`。

| 遍历顺序中的 bias | 回调 | 主程序实测结果 |
| --- | --- | --- |
| 10 | add_with_bias | 16 |
| 20 | multiply_with_bias | 28 |
| 30 | add_with_bias | 36 |

`run_tasks` 在 `task.c` 中按链表次序调用 `operation(a, b, context)`，打印上下文和结果，并推进遍历指针。
本次上下文契约是 `int*`，因此打印时使用 `*(int*)context`；这不是通用 `void*` 的自动类型识别。
当前输出未逐条换行只影响显示，不改变上述调用顺序和结果。

执行只借用节点和上下文，不出队、不释放节点，也不修改调用者的 `head`。
因此多次调用会重复遍历和执行；执行完再由调用者销毁。
主程序随后两次调用 `destroy_task_list(&head)`，两次检查均为 `head == NULL`。

## 4. 三层所有权与分配失败回滚

```text
TaskNode（malloc） -> Task（malloc） -> int context（malloc）
```

每个完整节点有三块独立堆内存，指针字段只是保存地址，不会自动释放所指对象。

```text
create_task_node
  -> 分配 TaskNode
  -> create_task
       -> 分配 Task
       -> prepare_task
            -> 分配 int context，复制 bias 值，设置回调
```

第 1 次分配失败：返回 NULL，没有已分配对象。
第 2 次分配失败：`create_task_node` 释放已分配节点。
第 3 次分配失败：`create_task` 先释放 Task，外层再释放节点。
成功后的销毁顺序为 context -> Task -> TaskNode，随后置空调用者指针。

`push_stack` 只在创建成功后修改原表，因此本次注册失败不会丢失已有任务。
`main` 的策略仍是单项失败时打印并跳过，保留成功任务；没有强制改成全量回滚或尾插。

## 5. 已取得的 push_stack 隔离验证证据

驱动：`/tmp/f01-push-stack-_l6d8xzs/harness.c`，直接引用真实 `task.c`，仅在临时驱动中包装 malloc/free。
编译使用 `-std=c11 -Wall -Wextra -Wpedantic -Werror`，编译和四次运行均退出 0。

| 检查 | 实际结果 |
| --- | --- |
| NULL head、NULL operation | 返回错误，0 次分配 |
| 合法空表 | 创建成功 |
| 自定义回调登记 | 保存指定地址，插入期间调用次数为 0 |
| 连续头插 bias 10/20/30 | 遍历为 30/20/10 |
| 驱动中显式调用自定义 a*b+bias | 结果 38/28/18，与主程序另一组回调区分 |
| 已有两节点后，第 1 次 malloc 失败 | 新增分配/释放 0/0，原表保留 |
| 已有两节点后，第 2 次 malloc 失败 | 新增分配/释放 1/1，原表保留 |
| 已有两节点后，第 3 次 malloc 失败 | 新增分配/释放 2/2，原表保留 |

失败检查核对原 head、next、Task/context 地址、bias 值和回调均不变。
失败三种情形最终分配/释放分别为 6/6、7/7、8/8；成功三节点为 9/9，存活分配均为 0，head 为 NULL。
跟踪器没有发现无效或重复 free。分配失败注入发生在 `/tmp` 驱动，不是主程序里新增的故障逻辑。
此前 ASan/UBSan 所需库缺失，未完成消毒器运行；失败注入及计数检查不等于 sanitizer 验证。

## 6. 口头验收中的纠正点

- `TaskNode* saved = head` 只复制地址；销毁 `&head` 不会自动把 `saved` 也置 NULL。
- `head` 被置空后，指向已释放节点的 `saved` 是悬空别名，不能再交给 `run_tasks` 使用。
- 提前 `free(context)` 会使后续回调或 `run_tasks` 的上下文打印访问失效对象；正常销毁时还可能再次 free 同一内存。
- 即使某个回调暂时不读 context，执行器仍可能读取它，不能据此声称提前释放安全。
- `prepare_task` 将 bias 的值复制进独立堆对象；之后修改原局部 bias，不会同步改变已登记任务的上下文值。
- 回调借用 context；释放责任仍属于拥有任务的一方，不能把借用理解成所有权转移。
- `static` 函数的内部链接限制按名字跨模块链接，不妨碍通过函数指针跨模块调用。
- `run_tasks` 不消费链表，再次调用会再次执行所有仍存活的任务。

综合输出题经纠正后的分析结果为 `28 / 16 / 35 / 18`；这是口头推演，不是本轮主程序实测 stdout。
起初漏算了两项，已解释执行器会重复遍历；输出预测仍需要巩固，不能把范围覆盖等同于熟练度。
本轮核心练习与纠正后的口头检查按课程要求通过。

## 7. 真实代码调用链与复现命令

```text
main.c:main -> task.c:push_stack -> create_task_node -> create_task -> prepare_task
main.c:main -> task.c:run_tasks -> task->operation
                                      -> main.c:multiply_with_bias（static）
                                      -> task.c:add_with_bias
main.c:main -> task.c:destroy_task_list -> destroy_task_node -> destroy_task
```

主工程本轮使用 `/tmp/f01-rebuild.9krDHR`，对应构建和运行命令：

```bash
cd /root/workspace/node-runtime-lab
cmake -S labs/00-foundations/c-minimum -B /tmp/f01-rebuild.9krDHR -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/f01-rebuild.9krDHR --clean-first
timeout 5s /tmp/f01-rebuild.9krDHR/f01-pointers
```

下面是本轮已执行的隔离检查命令；参数 0 为正常/契约检查，1/2/3 为在已有链表上注入对应分配失败。

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror /tmp/f01-push-stack-_l6d8xzs/harness.c -o /tmp/f01-push-stack-_l6d8xzs/harness
/tmp/f01-push-stack-_l6d8xzs/harness 0
/tmp/f01-push-stack-_l6d8xzs/harness 1
/tmp/f01-push-stack-_l6d8xzs/harness 2
/tmp/f01-push-stack-_l6d8xzs/harness 3
```

主工程也已完成严格重建与运行。临时驱动用于本机检查，不是提交到仓库的独立测试套件。

## 8. 下一步：F02 C++ 最小集

F02 从值复制与引用开始，已展示 `int copy` 和 `int& ref` 的对比题，但尚未作答或完成正式实验，不标记通过。
后续用 C++ 练习继续巩固声明/定义/调用一致、别名与生命周期，以及先预测再验证输出。
F01 已完成本轮验收；接下来继续学习，不额外扩张 C 最小集的收尾范围。
