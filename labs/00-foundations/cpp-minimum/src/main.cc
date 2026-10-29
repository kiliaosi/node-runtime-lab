#include <cstdio>
#include <iostream>
#include <memory>
#include <utility>

void test_scope();
void test_unique();
void test_move();
void test_transfer();
void test_shared_ptr();
void test_template();
void test_class_template();
void test_polymorphism();
void test_polymorphic_owner();
void test_lambda();
struct Trace {
  Trace() { std::puts("create"); }

  ~Trace() { std::puts("destroy"); }
};

template <typename T>
T larger(T a, T b) {
  return a > b ? a : b;
}

void custom_trace(std::unique_ptr<Trace> item);

template <typename T>
class ValueBox {
 public:
  explicit ValueBox(T value) : value_(value) {};
  T get() const { return this->value_; };
  void set(T value) { this->value_ = value; };

 private:
  T value_;
};

class Operation {
 public:
  virtual int run(int a, int b) const = 0;
  virtual ~Operation() { std::puts("Operation destroy"); };
};

class MultiplyOperation : public Operation {
 public:
  int run(int a, int b) const override { return a * b; };
  ~MultiplyOperation() override { std::puts("MultiplyOperation destroy"); }
};

class AddOperation : public Operation {
 public:
  int run(int a, int b) const override { return a + b; }
};

int calculate(const Operation& op, int a, int b) { return op.run(a, b); }

int main() {
  std::puts("scope");
  test_scope();

  std::puts("unique");
  test_unique();

  std::puts("move");

  test_move();

  std::puts("transfer");
  test_transfer();

  std::puts("shared_ptr test");
  test_shared_ptr();

  std::puts("template test");
  test_template();

  std::puts("class template test");
  test_class_template();

  std::puts("test calc");

  test_polymorphism();

  std::puts("test_polymorphic_owner");

  test_polymorphic_owner();

  std::puts("test_lambda");
  test_lambda();
}

void test_scope() {
  std::puts("A");

  {
    Trace t;
    std::puts("B");
  }

  std::puts("C");
}

void test_unique() {
  std::puts("A");

  {
    std::unique_ptr<Trace> p = std::make_unique<Trace>();
    std::puts("B");

    return;
  }
  std::puts("C");
}

void test_move() {
  std::unique_ptr<Trace> p = std::make_unique<Trace>();

  {
    std::unique_ptr<Trace> q = std::move(p);
    std::printf("p empty: %d\n", p == nullptr);
    std::printf("q empty: %d\n", q == nullptr);
  }

  std::puts("after inner scope");
}

void custom_trace(std::unique_ptr<Trace> item) {
  std::puts("inside custom_trace");
  std::printf("item empty: %d\n", item == nullptr);
}

void test_transfer() {
  std::unique_ptr<Trace> item = std::make_unique<Trace>();

  std::puts("before call");
  custom_trace(std::move(item));
  std::puts("after call");
  std::printf("caller item empty: %d\n", item == nullptr);
}

void test_shared_ptr() {
  std::shared_ptr<Trace> p = std::make_shared<Trace>();

  std::cout << p.use_count() << std::endl;

  {
    std::shared_ptr<Trace> q = p;
    std::cout << p.use_count() << std::endl;
    p.reset();

    std::cout << (p == nullptr) << std::endl;

    std::cout << p.use_count() << std::endl;

    std::cout << "after reset" << std::endl;
  }

  std::cout << "after inner scope" << std::endl;
}

void test_template() {
  std::cout << larger<int>(3, 7) << std::endl;
  std::cout << larger<double>(2.5, 1.2) << std::endl;
  std::cout << larger(8, 5) << std::endl;
}

void test_class_template() {
  ValueBox<int> a(10);
  ValueBox<float> b(2.5);

  std::cout << a.get() << std::endl;
  std::cout << b.get() << std::endl;

  a.set(20);
  std::cout << a.get() << std::endl;
}

void test_polymorphism() {
  MultiplyOperation mp;

  AddOperation add;

  int result = calculate(add, 2, 4);

  int result2 = calculate(mp, 2, 4);

  std::cout << result << std::endl;
  std::cout << result2 << std::endl;
}

void test_polymorphic_owner() {
  {
    std::unique_ptr<Operation> op = std::make_unique<MultiplyOperation>();

    int result = calculate(*op, 3, 4);
    std::cout << result << std::endl;
  }
  std::puts("after owner scope");
}

void test_lambda() {
  auto add = [](int a, int b) { return a + b; };
  auto multiply = [](int a, int b) { return a * b; };

  std::cout << add(4, 5) << std::endl;
  std::cout << multiply(4, 5) << std::endl;

  int num = 10;
  auto by_ref = [&num](int a, int b) { return a + b + num; };
  auto by_value = [num](int a, int b) { return a + b + num; };

  num = 20;

  std::cout << by_ref(4, 5) << std::endl;
  std::cout << by_value(4, 5) << std::endl;
}