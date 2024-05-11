#include "example_pool.h"

static inline std::chrono::time_point<std::chrono::steady_clock> getCurrentTimeMillis() {
    return std::chrono::steady_clock::now();
}

int main()
{
    ecs::example_pool();
    ecs::example_no_pool();
    return 0;
}
