#include <iostream>
#include <string>
#include <vector>
#include <utility> // For std::forward

struct field {
    union value_t {
        int integer;
        double decimal;
        std::string text;
        std::vector<uint8_t> blob;

        value_t() {} // Default constructor
        ~value_t() {} // Default destructor
    } value;

    enum class type { null, integer, decimal, blob, text } type = type::null;

    field() : type(type::null) {}

    explicit field(int val) : type(type::integer) {
        new(&value.integer) int(val);
    }

    explicit field(double val) : type(type::decimal) {
        new(&value.decimal) double(val);
    }

    field(const std::string& val) : type(type::text) {
        new(&value.text) std::string(val);
    }

    // Copy constructor
    field(const field& other) : type(other.type) {
        switch (type) {
            case type::integer:
                new(&value.integer) int(other.value.integer);
                break;
            case type::decimal:
                new(&value.decimal) double(other.value.decimal);
                break;
            case type::text:
                new(&value.text) std::string(other.value.text);
                break;
            default:
                break;
        }
    }

    // Destructor
    ~field() {
        if (type == type::text) {
            value.text.~basic_string();
        }
    }

    // Visit method
    template<typename Visitor>
    void visit(Visitor&& visitor) const {
        switch (type) {
            case type::integer:
                visitor(value.integer);
                break;
            case type::decimal:
                visitor(value.decimal);
                break;
            case type::text:
                visitor(value.text);
                break;
            default:
                // Handle other types or default case if necessary
                break;
        }
    }
};

// Example visitor
struct PrintVisitor {
    void operator()(int i) const { std::cout << "int: " << i << '\n'; }
    void operator()(double d) const { std::cout << "double: " << d << '\n'; }
    void operator()(const std::string& s) const { std::cout << "string: " << s << '\n'; }
};

int main() {
    field f1 (42);
    field f2 (3.14);
    field f3 ("Hello, World!");

    PrintVisitor printVisitor;

    f1.visit(printVisitor); // Outputs: int: 42
    f2.visit(printVisitor); // Outputs: double: 3.14
    f3.visit(printVisitor); // Outputs: string: Hello, World!

    return 0;
}
