#include <cstdio>
#include <iostream>
#include <memory>
#include <utility>

// Replace these four declarations with inline function bodies.
class File {
 public:
  explicit File(const char* path) {
    std::FILE* file = std::fopen(path, "w");
    this->file_ = file;
  };

  File(File&& other) noexcept {
    this->file_ = other.file_;
    other.file_ = nullptr;
  }

  ~File() {
    if (this->file_ == nullptr) return;
    int result = std::fclose(this->file_);
    if (result == 0) {
      this->file_ = nullptr;
    } else {
      printf("close file failed");
    }
  };
  bool valid() const { return this->file_ != nullptr; };
  bool write(const char* text) {
    if (this->file_ == nullptr) return false;

    auto result = std::fputs(text, this->file_);

    return result != EOF;
  };

  bool write_owned(File file) {
    if(!file.valid()) return false;

    return file.write("owned\n");
  };

  // One owner per file: copying is disabled for this exercise.
  File(const File&) = delete;
  File& operator=(const File&) = delete;

  File& operator=(File&& other) noexcept {
    if(this == &other) return *this;

    if(this->file_!=nullptr) {
      std::fclose(this->file_);
    }

    this->file_ = other.file_;
    other.file_=nullptr;

    return *this;
  }

 private:
  std::FILE* file_ = nullptr;
};

void test_file_scope();
void test_file_return();
void test_file_failure();
bool write_message(File& file);
void test_file_borrow();
void test_file_move();

void test_file_transfer();
void test_file_move_assign();

int main() {
  // TODO: Call the three exercises as described in FILE_RAII.md.
  std::puts("normal");
  test_file_scope();

  std::puts("early");
  test_file_return();
  std::puts("after return");

  std::puts("failure");
  test_file_failure();
  std::puts("after failure");


  test_file_borrow();
  std::puts("after borrow test");


  test_file_move();

  std::puts("after move test");

  test_file_transfer();

  std::puts("after transfer test");

  test_file_move_assign();
  std::puts("after move assign test");
  return 0;
}

void test_file_scope() {
  {
    std::unique_ptr<File> f = std::make_unique<File>("build/f02-cpp-minimum/raii-normal.txt");

    if (!f->valid()) {
      return;
    }

    if(!f->write("normal\n")) {
      std::printf("write faild");
      return;
    };
  }

  std::printf("after scope");
}

void test_file_return() {
  std::unique_ptr<File> f = std::make_unique<File>("build/f02-cpp-minimum/raii-early.txt");

  if (!f->valid()) {
    return;
  }

 if(! f->write("early\n")) {
  std::printf("write faild");
  return;
 };

  std::printf("before return");
  return;
}

void test_file_failure() {
  std::unique_ptr<File> f = std::make_unique<File>(".");

  std::cout << f->valid() << std::endl;

  std::cout << f->write("test error") << std::endl;
}

bool write_message(File& file) {
  if(!file.valid()) return false;

  return file.write("borrowed\n");
}

void test_file_borrow(){
  std::unique_ptr<File> f = std::make_unique<File>("build/f02-cpp-minimum/raii-borrow.txt");

  bool result = write_message(*f);

  if(!result) return;
   
  bool result2 = f->write("owner\n"); 

  if(!result2) return;

  std::printf("after borrow");
}

void test_file_move() {
  std::unique_ptr<File> first = std::make_unique<File>("build/f02-cpp-minimum/raii-move.txt");

  if(!first->valid()) return;


  std::unique_ptr<File> second = std::make_unique<File>(std::move(*first));

  std::cout<<first->valid()<<std::endl;
  std::cout<<second->valid()<<std::endl;

  bool result = second->write("moved\n");

  std::cout<<result<<std::endl;

}

void test_file_transfer() {
  std::unique_ptr<File> file = std::make_unique<File>("build/f02-cpp-minimum/raii-transfer.txt");

  if(!file->valid()) return;

  bool result = file->write_owned(std::move(*file));

  std::cout<<result<<std::endl;

  std::cout<<(file!= nullptr)<<std::endl;

  std::cout<<file->valid()<<std::endl;
}

void test_file_move_assign(){
  std::unique_ptr<File> target = std::make_unique<File>("build/f02-cpp-minimum/raii-assign-old.txt");

  if(!target->valid()) return;

  std::unique_ptr<File> source = std::make_unique<File>("build/f02-cpp-minimum/raii-assign-new.txt");

  if(!source->valid()) return;

  if(!target->write("old\n")) {
    std::cout<<"write target faild"<<std::endl;
  }

  if(!source->write("before\n")) {
    std::cout<<"write source faild"<<std::endl;
  }

  // 开始移动

  *target = std::move(*source);

  std::cout<<(target->valid())<<std::endl;
  std::cout<<(source->valid())<<std::endl;

  bool result = target->write("after\n");

  std::cout<<result<<std::endl;
}