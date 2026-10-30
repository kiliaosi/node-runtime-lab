// Repaired ownership examples. Verify them with the Valgrind tasks.
#include <cstdlib>
#include <cstring>
#include <iostream>

class Buffer {
 public:
  Buffer() : data_(static_cast<int*>(std::malloc(sizeof(int)))) {
    if (data_ != nullptr) *data_ = 42;
  }

  // This buffer owns its allocation; copying would duplicate ownership.
  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;

  ~Buffer() {
    std::cout << "free " << static_cast<void*>(data_) << std::endl;
    std::free(data_);
  }

  bool valid() const { return data_ != nullptr; }
  int* data() const { return data_; }  // Borrowed pointer; Buffer owns the memory.

 private:
  int* data_;
};

int test_double_free() {
  Buffer first;
  if (!first.valid()) {
    std::cerr << "allocation failed" << std::endl;
    return 1;
  }

  {
    Buffer& second = first;
    std::cout << "copy value: " << *second.data() << std::endl;
  }

  std::cout << "after inner scope" << std::endl;
  return 0;
}

int test_use_after_free() {
  int* saved = nullptr;
  Buffer owner;

  {
    if (!owner.valid()) {
      std::cerr << "allocation failed" << std::endl;
      return 1;
    }
    saved = owner.data();
  }

  // The owner is still alive, so the borrowed pointer remains valid.
  std::cout << "borrowed value: " << *saved << std::endl;
  return 0;
}

int main(int argc, char* argv[]) {
  if (argc == 1) {
    std::cout << "Select a case through the VS Code Valgrind tasks:"
              << "\n  double-free\n  use-after-free" << std::endl;
    return 0;
  }
  if (argc != 2) return 2;
  if (std::strcmp(argv[1], "double-free") == 0) return test_double_free();
  if (std::strcmp(argv[1], "use-after-free") == 0) return test_use_after_free();
  std::cerr << "unknown case" << std::endl;
  return 2;
}
