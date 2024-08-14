#include <fmt/format.h>

#include <string_view>
#include <variant>

struct SpaceObject {
  constexpr virtual ~SpaceObject() = default;
  [[nodiscard]] virtual constexpr auto get_name() const noexcept
      -> std::string_view = 0;
  int x;
  int y;
};

struct Craft : SpaceObject {
  [[nodiscard]] constexpr std::string_view get_name() const noexcept override {
    return "Craft";
  }
};
struct Asteroid : SpaceObject {
  [[nodiscard]] constexpr std::string_view get_name() const noexcept override {
    return "Asteroid";
  }
};

std::unique_ptr<SpaceObject> factory();

template <typename... Callable>
struct visitor : Callable... {
  using Callable::operator()...;
};

template <class T1, class T2>
struct collide {
 using vtype = std::variant<T1, T2>;
 constexpr collide(const vtype& v1,const vtype& v2) : m_v1(v1),m_v2(v2) { }
 constexpr operator int() noexcept {
    std::visit(
        [this](const auto &lhs, const auto &rhs) {
          if (lhs.x == rhs.x && lhs.y == rhs.y) {
            this->result += lhs.x + rhs.y;
          }
        },
        m_v1, m_v2);    
    return result;
  }
  int result = 0;
  const vtype& m_v1;
  const vtype& m_v2;
};

std::vector<std::variant<Craft, Asteroid>> get_objects();

void process_collisions(const std::variant<Craft, Asteroid> &obj) {
  int result = 0;
  for (const auto &other : get_objects()) {
    result += collide(obj, other);
  }
}
