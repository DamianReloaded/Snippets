#include <iostream>
#include <variant>
#include <string_view>

int main() {
    auto print = [](auto y){std::visit([](auto x){
        struct print {
            print (int i) { printf("%i\n",i);}
            print (float f) { printf("%f\n",f);}
            print (std::string_view sv) {  printf("%.*s\n", static_cast<int>(sv.size()), sv.data()); } 
        } print(x);
    }, y);};
    std::variant<int, float, std::string_view> var = 123;
    print(var);
    var = "Hello World!";
    print(var);    
}
