#include <variant>
#include <fmt/format.h>
#include <string_view>

std::variant<int, float, std::string_view> get_variant()
{
  return 4.2f;
}

template<typename ... Callable>
struct visitor : Callable... {
  using Callable::operator()...;
};

int main()
{
  const auto value = get_variant();
  std::visit(
    visitor{[](int i){fmt::print("Int: {}\n", i);}, 
            [](float f){fmt::print("Float: {}\n", f);},
            [](std::string_view sv){fmt::print("SV: {}\n", sv);}}
    , value);
}