//#include "source/SimpleHtmlServer/simple_html_server.h"
//#include "source/QLearning/qlearning01.h"
//#include "source/SimpleWebsockets/simple_websocket_server.h"
//#include <chrono>
//#include <thread>
#include "ecs/example_ecs.h"
#include "ecs/example_pool.h"

static inline std::chrono::time_point<std::chrono::steady_clock> getCurrentTimeMillis() {
    return std::chrono::steady_clock::now();
}

int main()
{
//    ecs::example_ecs();
    ecs::example_pool();
    ecs::example_no_pool();
    return 0;
}
