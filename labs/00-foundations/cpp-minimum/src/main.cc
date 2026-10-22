#include <cstdio>
#include <memory>
#include <utility>

void test_scope();
void test_unique();
void test_move();
void test_transfer();


struct Trace {
    Trace(){
        std::puts("create");
    }

    ~Trace() {
        std::puts("destroy");
    }
};

void custom_trace(std::unique_ptr<Trace> item);


int main() {
    std::puts("scope");
    test_scope();

    std::puts("unique");
    test_unique();

    std::puts("move");

    test_move();

    std::puts("transfer");
    test_transfer();

}


void test_scope(){
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
        std::unique_ptr<Trace>  q = std::move(p);
        std::printf("p empty: %d\n", p == nullptr);
        std::printf("q empty: %d\n", q == nullptr);
    }

    std::puts("after inner scope");
}

void custom_trace(std::unique_ptr<Trace> item) {
    std::puts("inside custom_trace");
std::printf("item empty: %d\n", item == nullptr);
}

void test_transfer(){
  std::unique_ptr<Trace> item = std::make_unique<Trace>();

  std::puts("before call");
custom_trace(std::move(item));
std::puts("after call");
std::printf("caller item empty: %d\n", item == nullptr);
}
