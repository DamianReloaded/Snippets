#include <string_view>
#include <iostream>
#include <array>

        // Generic constexpr lookup from string to enum
    template <typename EnumId, size_t N>
    [[nodiscard]]
    constexpr EnumId FromString(
        std::string_view name,
        const std::pair<EnumId, std::string_view> (&map)[N],
        EnumId defaultValue = EnumId{})
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (map[i].second == name)
                return map[i].first;
        }
        return defaultValue;
    }

    // Generic constexpr lookup from enum to string
    template <typename EnumId, size_t N>
    [[nodiscard]]
    constexpr std::string_view ToString(
        EnumId id,
        const std::pair<EnumId, std::string_view> (&map)[N],
        std::string_view defaultValue = "none")
    {
        for (size_t i = 0; i < N; ++i)
        {
            if (map[i].first == id)
                return map[i].second;
        }
        return defaultValue;
    }

    // Helper struct to bundle map with interface
    template <typename EnumId, size_t N>
    struct EnumString
    {
        constexpr EnumString(const std::pair<EnumId, std::string_view> (&m)[N])
            : map(m)
        {}

        [[nodiscard]] constexpr EnumId FromString(std::string_view name, EnumId defaultValue = EnumId{}) const
        {
            for (size_t i = 0; i < N; ++i)
            {
                if (map[i].second == name)
                    return map[i].first;
            }
            return defaultValue;
        }

        [[nodiscard]] constexpr std::string_view ToString(EnumId id, std::string_view defaultValue = "none") const
        {
            for (size_t i = 0; i < N; ++i)
            {
                if (map[i].first == id)
                    return map[i].second;
            }
            return defaultValue;
        }

        [[nodiscard]] constexpr size_t ToSize(EnumId id) const
        {
            return static_cast<size_t>(id);
        }

        [[nodiscard]] constexpr EnumId FromSize(size_t index, EnumId defaultValue = EnumId{}) const
        {
            if (index < N)
                return map[index].first;
            return defaultValue;
        }

        // Expose the entire mapping for iteration
        [[nodiscard]] constexpr const std::pair<EnumId, std::string_view> * GetMap() const
        {
            return map;
        }

        [[nodiscard]] constexpr size_t Size() const
        {
            return N;
        }

        // Alternatively, a foreach helper:
        template <typename Func>
        constexpr void ForEach(Func&& f) const
        {
            for (size_t i = 0; i < N; ++i)
                f(map[i].first, map[i].second);
        }

        const std::pair<EnumId, std::string_view> (&map)[N];
    };

    #define DefineEnumToStringEntry(EnumClass, e) { EnumClass::e, #e }
    #define DefineEnumToStringEntryCustom(EnumClass, e, str) { EnumClass::e, str }

    enum class ObjectId
    {
        None, Coin, DoorExit01, Enemy01, Enemy02, Platform01, Player, CoinSpawner
    };

    constexpr std::pair<ObjectId, std::string_view> ObjectIdMap[] = {
        DefineEnumToStringEntryCustom(ObjectId, Coin, "coin"),
        DefineEnumToStringEntry(ObjectId, DoorExit01),
        DefineEnumToStringEntry(ObjectId, Enemy01),
        DefineEnumToStringEntry(ObjectId, Enemy02),
        DefineEnumToStringEntry(ObjectId, Platform01),
        DefineEnumToStringEntry(ObjectId, Player),
        DefineEnumToStringEntry(ObjectId, CoinSpawner),
        DefineEnumToStringEntry(ObjectId, None)
    };

    // One instance per enum to provide to_string / from_string interface
    constexpr EnumString ObjectIdConvert(ObjectIdMap);

int main()
{
    size_t b= 0;
    ObjectIdConvert.ForEach([&b](ObjectId id, std::string_view name) {
            b += ObjectIdConvert.ToSize(id);
            std::cout <<name <<" ,";
        });

    
    return b;
}