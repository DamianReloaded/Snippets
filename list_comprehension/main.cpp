#include <array>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <string>

// Mimic the handyness of list comprehension
// result = [x*x for x in nums]

template <typename T, size_t S>
class into : public std::array<T,S>
{
    public:
        into (std::array<T,S>& input, auto func) 
        {
            for (size_t i=0; i<this->size(); i++) (*this)[i] = func(input[i]);
        }
        std::array<T,S> operator ()() { return *this; }
};

template<typename T, template<typename...> class C, typename... Args>
class in 
{
    public:
        in (C<T, Args...>& input, auto func) 
        {
            for (auto inp=input.begin(); inp!=input.end(); inp++) (void)items.insert(items.end(),func(*inp));
        }
        const auto&             operator [] (const size_t& i) { auto iter = items.begin(); std::advance(iter,i); return *iter; }
        C<T, Args...>           operator () ()                { return items; }
    private:
        C<T, Args...> items;
};

#define each(x) [](auto x) 
#define is(x) {return x;}

using namespace std;
int main()
{
    switch(2)
    {
        case 1:
        {
            array nums = {1,2,3,4};
            auto result = into(nums, each (x) is(x*x)); 
            return result[1];
        }   
        case 2:
        {
            map<int,string> nums = { {1,"one"}, {2,"two"}, {3,"three"}, {4,"four"} }; 
            auto result = in(nums, each(x) is(make_pair(x.first*x.first,x.second)));
            return result[1].first;
        }
        case 3:
        {
            vector<int> nums = {1,2,3,4};
            auto result = in(nums, each(x) is((true)?x*x:x) );
            return result[1];
        }
        case 4:
        {
            set nums = {1,2,3,4};
            auto result = in(nums, each(x) is((true)?x*x:x) );
            return result[1];
        }
        case 5:
        {
            vector<int> nums = {1,2,3,4};
            auto result = in(nums, each(x) is(x*x) );
            return result[1];
        }
        case 6:
        {
            deque<int> nums = {1,2,3,4};
            auto result = in(nums, each(x){return x*x;});
            return result[1];
        }
        case 7:
        {
            string letters = "hello";
            auto result = in(letters, each(x){return toupper(x);});
            return result[1];
        }
    }
    return 0;
}