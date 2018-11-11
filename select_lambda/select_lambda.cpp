#include <deque>
#include <cstdio>
#include <algorithm>

struct table
{
    int id;
    const char* name;
};

template <class C, class T>
class query_base : public C
{
    public:
        std::deque<const T*> result;
        template <typename F>
        std::deque<const T*>& operator () (const F& where)
        {
            result.clear();
            const C& items = *this;
            for(auto t : items) 
                if (where(t)) 
                    result.push_back(&t);
            return result;
        }
};

template <class T>
class query : public query_base<std::deque<T>,T> {};

int main()
{
    query<table> tables;
    tables.push_back(table{0,"cero"});
    tables.push_back(table{1,"uno"});
    tables.push_back(table{0,"dos"});

    auto& q = tables( [](auto& t){ return t.id==1;} );
    for (auto t:q) printf(t->name);

    return 0;
}
