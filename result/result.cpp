/*
	result class to use as return value.
	contains an error member you can set when there is an error.
	uses a compile time allocated string implementation to avoid std::string allocations
*/

#include <array>
#include <cstring>
#include <string_view>

template <class T, int Tsize>
class str
{
    public:
        str(){}
        str (const T* _val)
        {
            size_t _size = strlen(_val);
            size_t size = (_size<Tsize)?_size:Tsize;
            std::copy(_val, &_val[size], &buffer[0] );
            std::fill(&buffer[size],&buffer[size]+sizeof(T), 0);
        }
        constexpr std::basic_string_view<T>  string() const { return &buffer[0]; }
        constexpr operator std::basic_string_view<T>  () const { return &buffer[0]; }
    protected:

        std::array<T,Tsize+sizeof(T)> buffer;
};

struct error
{
                        error           () : number(0) {}
                        error           (const int& _number) : number(_number) {}
                        error           (const str<char,100>& _description) : number(0), description(_description) {}
                        error           (const int& _number, const str<char,100>& _description) : number(_number), description(_description) {}
    const int          number;
    const str<char,100>  description;
};

template <class T>
struct result
{
                        result (T _value) : value(_value) {}
                        result (T _value, const error& _error) : value(_value), err(_error) {}
                        constexpr operator T  () const { return value; }
    T                value;
    const error      err;
};

result<bool> test(int pos)
{
    if (pos==0) return {false,{33,"error" }};
    return {true,{44,"ok" }};
}

int main()
{
    auto val = test(0);
    return val.err.number;
}