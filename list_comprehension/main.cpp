#include <array>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <list>
#include <string>

template <typename T, size_t S>
class into : public std::array<T,S>
{
  public:
    into (std::array<T,S>& input, auto func)
    {
      for (size_t i=0; i<this->size(); i++)
      {
        (*this)[i] = func(input[i]);
      }
    }

    into (std::array<T,S>& input, auto condition, auto func)
    {
      for (size_t i=0; i<this->size(); i++)
      {
        if (!condition(input[i])) continue;
        (*this)[i] = func(input[i]);
      }
    }

};

template<typename T, template<typename...> class C, typename... Args>
class in
{
  public:
  in (C<T, Args...>& input, auto condition, auto func)
  {
    for (auto inp=input.begin(); inp!=input.end(); inp++)
    {
      if (!condition(*inp)) continue;
      (void)items.insert(items.end(),func(*inp));
    }
  }

  in (C<T, Args...>& input, auto func)
  {
    for (auto inp=input.begin(); inp!=input.end(); inp++)
    {
      (void)items.insert(items.end(),func(*inp));
    }
  }

  auto        begin        () const                { return items.begin(); }
  auto        end          () const                { return items.end(); }
  auto        size         () const                { return items.size(); }
  const auto& operator []  (const size_t& i) const { auto iter = items.begin(); std::advance(iter,i); return *iter; }
  operator    auto         () const                { return items; }

  private:
    C<T, Args...> items;
};

#define when(x,c,f) [](const auto& x) {return bool(c);}, [](const auto& x) {return f;}
#define each(x,f) [](const auto& x) {return f;}

using namespace std;
int main()
{
  switch(6)
  {
    case 1:
    {
      array nums = {1,2,3,4};
      auto result = into(nums, when(x, x<3, x*x));
      return result[1];
    }   
    case 2:
    {
      string letters = "hello";
      auto result = in(letters, each(x, toupper(x)));
      return result[1];
    }
    case 3:
    {
      list nums = {1,2,3,4};
      auto result = in(nums, each(x, (x>2)?x*x:x)); 
      return result[1];
    }
    case 4:
    {
      set nums = {1,2,3,4};
      auto result = in(nums, when(x, x<3, x*x));
      return result.size();
    }
    case 5:
    {
      deque nums = {1,2,3,4};
      auto result = in(nums, each(x, x*x));
      return result[1];
    }
    case 6:
    {
      vector nums = {1,2,3,4};
      auto result = in(nums, each(x, (true)?x*x:x));
      return result[1];
    }    
    case 7:
    {
      map<int,string> nums = { {1,"one"}, {2,"two"}, {3,"three"}, {4,"four"} }; 
      auto result = in(nums, each(x, make_pair(x.first*x.first,x.second)));
      return result[1].first;
    }
  }
  return 0;
}
