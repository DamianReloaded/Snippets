#include <cstdint>
#include <cstdio>

class test
{
    public:
        template <std::size_t size>
        explicit consteval test(const char (&input)[size]) : num(0)
        {
            for (const char c : input) if (c=='X') num++;
        }
        operator const size_t () const {return num;}
        std::size_t num;
};

int main()
{   
    return test("XXXX");
}