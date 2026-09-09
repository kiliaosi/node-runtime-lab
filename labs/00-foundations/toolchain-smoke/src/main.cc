#include <iostream>
#include <string>

struct Task {
  std::string name;
  int value;
};

using Callback = void (*)(void* context);

void run_callback(Callback callback, void* context) {
  callback(context);
}

void print_task(void* context) {
  auto* task = static_cast<Task*>(context);
  std::cout << "task=" << task->name << " value=" << task->value << '\n';
}

int main() {
  Task task{"toolchain-smoke", 42};
  run_callback(print_task, &task);
  return 0;
}
