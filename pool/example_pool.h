#pragma once
#include "entity.h"
#include <iostream>
#include <chrono>
#include "Pool.h" // Include your Pool class header file

#include <windows.h>
static inline std::chrono::time_point<std::chrono::steady_clock> getCurrentTimeMillis();


namespace ecs
{
    struct user
    {
        user(){}
        user(std::string p_name) : name(p_name) {}
        ~user()
        {

        }

        std::string name;
        int age;
        double height;
        bool enabled;
    };

    const int numItems = 100000; // Adjust this as needed

    int example_pool()
    {
        // Create an instance of your Pool class
        ecs::Pool<user> pool((size_t)(numItems*1)); // Replace MyType and BufferSize with actual types and sizes

        std::vector<std::shared_ptr<user>> v;
        v.reserve(numItems);

        auto startTime = getCurrentTimeMillis();
        // Allocate items in the pool
        for (int i = 0; i < numItems; ++i) {
            v.push_back( pool.MakeSharedPtr("") );
        }

        for (int i=0;i<10;i++)
        {
            // Iterate over the allocated items
            for (user* item : pool) {
                printf(item->name.c_str());
            }
        }
        auto endTime = getCurrentTimeMillis();

        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);
        std::cout << "Pool test:\t" << duration.count() << " nanoseconds." << std::endl;

        std::cout << "used chunks:\t" << pool.used_chunks() << std::endl;


        return 0;
    }

    int example_no_pool()
    {
        std::vector<std::shared_ptr<user>> v;
        v.reserve(numItems);

        auto startTime = getCurrentTimeMillis();
        // Allocate items in the pool
        for (int i = 0; i < numItems; ++i) {
            v.push_back( std::make_shared<user>("") );
        }
        for (int i=0;i<10;i++)
        {
            // Iterate over the allocated items
            for (auto item : v) {
                printf(item->name.c_str());
            }
        }
        auto endTime = getCurrentTimeMillis();

        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);
        std::cout << "No Pool test:\t" << duration.count() << " nanoseconds." << std::endl;

        return 0;
    }
}


