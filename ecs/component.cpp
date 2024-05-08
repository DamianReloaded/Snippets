#include <iostream>
#include <cstdint>
#include <vector>

namespace ecs
{
    struct icomponent
    {
        static size_t next_id() { static size_t id=0; return id++; }
    };

    template<class T>
    struct component : public icomponent
    {
        static const size_t id;
        static const size_t size = sizeof(T);

        static inline size_t allocate(std::vector<uint8_t>& memory, icomponent* base)
        {
            size_t index = memory.size();
            memory.resize(index+T::size);
            new (&memory[index]) T(*(T*)base);
            return index;
        }
    };

    template<class T> const size_t component<T>::id = icomponent::next_id();
}

struct test : ecs::component<test>
{
    int x,y;
};

struct tast : ecs::component<tast>
{
    std::string name;
};

int main()
{
    //return run_qlearning01();
    //test_simple_http_server();
    //test_simple_websockets();

    test t; t.x=123; t.y=345;
    std::vector<uint8_t> memory;
    size_t index = ecs::component<test>::allocate(memory, &t);
    index = ecs::component<test>::allocate(memory, &t);

    test* t2 = (test*)&memory[index];
    std::cout <<"id: "<< test::id <<"\n";
    std::cout <<"index: "<< index <<"\n";
    std::cout <<"t2.x: "<< t2->x <<"\n";
    std::cout <<"t2.y: "<< t2->y <<"\n";


    tast ta; ta.name = "hello world";
    std::vector<uint8_t> memory_ta;
    size_t index_ta = ecs::component<tast>::allocate(memory_ta, &ta);
    tast* ta2 = (tast*)&memory_ta[index_ta];
    std::cout <<"id: "<< tast::id <<"\n";
    std::cout <<"index: "<< index_ta <<"\n";
    std::cout <<"ta.name: "<< ta2->name <<"\n";


    return 0;
}
