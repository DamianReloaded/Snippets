# C++26 Compile-Time Enum Reflection Library

A zero-overhead enum reflection utility for C++26 using the standard reflection facilities.

This library automatically generates enum-to-string and string-to-enum conversion tables at compile time. The enum declaration itself becomes the single source of truth.

No macros.  
No manual lookup tables.  
No runtime registration.  

---

## Features

- Compile-time enum reflection
- Automatic enum-to-string conversion
- Automatic string-to-enum conversion
- Enum iteration
- No duplicated metadata
- No runtime initialization
- Fully type-safe
- Works with any enum class
- C++26 reflection based

---

## Example

```cpp
#include <iostream>


int main()
{
    enum class ObjectId
    {
        enum0,
        enum1,
        enum2,
        enum3,
        enum4,
        enum5,
        enum6
    };


    using ObjectIdInfo =
        EnumInfo<ObjectId>;


    std::cout
        << "Enum count: "
        << ObjectIdInfo::Size()
        << '\n';


    ObjectId id =
        ObjectIdInfo::MakeEnum(
            "enum5");


    std::cout
        << "enum5 = "
        << static_cast<int>(id)
        << '\n';


    std::cout
        << ObjectIdInfo::ToString(
            ObjectId::enum3)
        << '\n';
}