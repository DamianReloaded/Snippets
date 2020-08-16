#include <array>
#include <algorithm>

template <typename Key, typename Value, std::size_t Size>
struct cxmap {
    std::array<std::pair<Key,Value>, Size> data;

    [[nodiscard]] constexpr Value at (const Key &key) const 
    {
        const auto itr = std::find_if(begin(data), end(data), [&key](const auto &v) { return v.first == key; });
        if (itr != end(data))
        {
            return itr->second;
        }
        else
        {
            throw std::range_error("Not Found");
        }
    }
};
struct config {
    enum struct id
    {
        value0, value1, value2, max
    };
    
    static id map(std::string_view key)
    {
        cxmap<std::string_view,config::id,size_t(config::id::max)> maps = {
            std::make_pair("value0",id::value0),
            std::make_pair("value1",id::value1),
            std::make_pair("value2",id::value2)
        };

        return maps.at(key);
    }
};

int main()
{
    return (int)config::map("value1");
}