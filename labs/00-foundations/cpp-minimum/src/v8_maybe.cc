#include <v8-local-handle.h>
#include <v8-maybe.h>

#include <iostream>

void test_maybe();
void test_maybe_local();

int main() {
  std::cout << "maybe test" << std::endl;
  test_maybe();
  test_maybe_local();
}

void test_maybe() {
  // TODO: Create a Just<int> and a Nothing<int>.
  // Print their states and their FromMaybe(-1) results.

  v8::Maybe<int> value = v8::Just<int>(42);

  v8::Maybe<int> empty = v8::Nothing<int>();

  // 打印1
  std::cout << value.IsJust() << std::endl;
  // 打印42
  std::cout << value.FromMaybe(-1) << std::endl;

  // 打印1
  std::cout << empty.IsJust() << std::endl;
  // 打印-1
  std::cout << empty.FromMaybe(-1) << std::endl;

  // 取值
  int out = -1;

  bool result = value.To(&out);

  std::cout << "是否取值成功" << result << std::endl;
  std::cout << "值:" << out << std::endl;

  out = -99;

  bool result2 = value.To(&out);
  std::cout << "修改值" << result2 << out << std::endl;

  out = -99;
  bool result3 = empty.To(&out);

  std::cout << "empty to: " << result3 << " " << out << std::endl;
}

void test_maybe_local() {
  v8::Local<v8::String> text;

  v8::MaybeLocal<v8::String> result;

  bool is_to = result.ToLocal(&text);

  std::cout << is_to << std::endl;
}