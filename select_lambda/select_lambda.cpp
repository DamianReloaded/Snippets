#include <deque>

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

#define where(x,w) [](auto& x) { return w; }

int main()
{
    struct record
    {
        int id;
        float value;
    };

    table<record> records;
    records.insert({1,1.123});
    records.insert({2,2.234});
    records.insert({3,3.345});
    auto results = records.select( where(r, r.id==2) );

    return results.rows[0].id;
}
