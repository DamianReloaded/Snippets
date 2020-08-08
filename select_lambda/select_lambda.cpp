#include <deque>
#include <cstdio>
#include <string>
#include <algorithm>

struct record
{
    int id;
    std::string name;
};

template <class T>
class table
{
    public:
        table<T> select (const auto& where)
        {
            table results;
            for(auto t : rows) if (where(t)) results.insert(t);
            return results;
        }
        void insert(const T& _record) {rows.push_back(_record);}

        std::deque<T> rows;
};

#define with(x,w) [](auto& x) { return w; }

int main()
{
    table<record> records;
    auto results = records.select( with(r, r.id==1) );
    for (auto t : results.rows) printf(t.name.c_str());
    return 0;
}