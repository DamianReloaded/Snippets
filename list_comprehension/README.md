# List comprehension in c++

### An attempt to mimic the handyness of list comprehension in python
### It uses macros tho :/

> PYTHON
```
nums = [1,2,3,4]
result = [x*x for x in nums]
```    
> C++ with macros (each, is)
```
set nums = {1,2,3,4};
auto result = in( nums, each(x) is(x*x) );
```
> C++ with lambda instead of macros
```
auto result = in(nums, [](auto x) { return x*x; } );
```
