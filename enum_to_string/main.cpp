#include <array>
#include <iostream>
#include <meta>
#include <string_view>

// ============================================================
// ENUM ENTRY
// ============================================================

template<typename Enum>
struct EnumEntry
{
    Enum value;
    std::string_view name;
};

// ============================================================
// REFLECTION
// ============================================================

template<typename Enum>
constexpr auto GetEnumerators()
{
    return std::define_static_array(
        std::meta::enumerators_of(
            ^^Enum));
}

// ============================================================
// ENUMERATOR ITERATION
// ============================================================

template<typename Enum, typename Function>
consteval void ForEachEnumerator(
    Function&& function)
{
    constexpr auto enumerators =
        GetEnumerators<Enum>();

    template for
    (
        constexpr auto enumerator :
        [: std::meta::reflect_constant_array(
            enumerators) :]
    )
    {
        function.template operator()<enumerator>();
    }
}

// ============================================================
// TABLE BUILDER
// ============================================================

template<typename Enum, size_t Size>
struct EnumTableBuilder
{
    std::array<
        EnumEntry<Enum>,
        Size>& entries;

    size_t& index;

    template<auto Enumerator>
    consteval void operator()() const
    {
        entries[index++] =
        {
            [:Enumerator:],
            std::meta::identifier_of(
                Enumerator)
        };
    }
};

// ============================================================
// TABLE GENERATION
// ============================================================

template<typename Enum>
consteval auto GenerateEnumTable()
{
    constexpr auto enumerators =
        GetEnumerators<Enum>();

    std::array<
        EnumEntry<Enum>,
        enumerators.size()> entries{};

    size_t index = 0;

    ForEachEnumerator<Enum>(
        EnumTableBuilder
        <
            Enum,
            enumerators.size()
        >
        {
            entries,
            index
        });

    return entries;
}


// ============================================================
// ENUM INFO
// ============================================================

template<typename Enum>
struct EnumInfo
{
    static constexpr auto entries =
        GenerateEnumTable<Enum>();


    static constexpr std::string_view ToString(
        Enum value,
        std::string_view fallback = "none")
    {
        for(auto&& entry : entries)
        {
            if(entry.value == value)
                return entry.name;
        }

        return fallback;
    }

    static constexpr Enum FromString(
        std::string_view name,
        Enum fallback = Enum{})
    {
        for(auto&& entry : entries)
        {
            if(entry.name == name)
                return entry.value;
        }

        return fallback;
    }

    static constexpr Enum MakeEnum(
        std::string_view name,
        Enum fallback = Enum{})
    {
        return FromString(
            name,
            fallback);
    }

    static constexpr size_t Size()
    {
        return entries.size();
    }

    template<typename Function>
    static constexpr void ForEach(
        Function&& function)
    {
        for(auto&& entry : entries)
        {
            function(
                entry.value,
                entry.name);
        }
    }
};

// ============================================================
// GENERIC UTILITIES
// ============================================================

struct PrintEnumEntry
{
    template<typename Enum>
    constexpr void operator()(
        Enum value,
        std::string_view name) const
    {
        std::cout
            << static_cast<int>(value)
            << " -> "
            << name
            << '\n';
    }
};

// ============================================================
// MAIN
// ============================================================

int main()
{
    enum class ObjectId { enum0, enum1, enum2, enum3, enum4, enum5, enum6};
    using ObjectIdInfo = EnumInfo<ObjectId>;
    
    std::cout << "Enum size: " << ObjectIdInfo::Size() << "\n\n";

    std::cout << "Lookup:\n";
    ObjectId id = ObjectIdInfo::FromString( "enum5");
    std::cout << "enum5 = " << static_cast<int>(id) << '\n';
    std::cout << "enum3 = " << ObjectIdInfo::ToString(ObjectId::enum3) << '\n';

    std::cout << "\nOperator lookup:\n";
    ObjectId id2 = ObjectIdInfo::MakeEnum("enum6");
    std::cout << "enum6 = " << static_cast<int>(id2) << '\n';

    std::cout << "\nList Enums:\n";
    ObjectIdInfo::ForEach(PrintEnumEntry{});
    std::cin.get();
}