#include <iostream>

// Forward declaration
template<bool AgeSet>
class Builder;

struct Person {
private:
    int age;
    int height;

    Person(int a, int h) : age(a), height(h) {}

    template<bool> friend class Builder;

public:
    int getAge() const { return age; }
    int getHeight() const { return height; }
};

template<bool AgeSet>
class Builder {
    int age{};
    int height{};

    // Allow all specializations to access each other
    template<bool> friend class Builder;

public:
    Builder() = default;

    // Private constructor for internal use
    Builder(int a, int h) : age(a), height(h) {}

    // Allowed only if age not set
    auto setAge(int a) const requires (!AgeSet) {
        return Builder<true>{a, height};
    }

    // When AgeSet == true → trigger error
    auto setAge(int) const requires (AgeSet) {
        static_assert(!AgeSet, "Error: setAge() can only be called once");
        return *this;
    }

    auto setHeight(int h) const {
        return Builder<AgeSet>{age, h};
    }

    Person build() const {
        static_assert(AgeSet, "Error: age must be set before build()");
        return Person(age, height);
    }
};

int main() {
    Builder<false> b;

    auto person = b.setAge(25)
                   .setHeight(180)
                   .build();

    std::cout << "Age: " << person.getAge()
              << ", Height: " << person.getHeight() << '\n';

    // Compile-time error if uncommented:
    /*
    auto bad = b.setAge(30)
                .setHeight(170)
                .setAge(40);
    */

    return 0;
}