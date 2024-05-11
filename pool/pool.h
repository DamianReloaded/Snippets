#pragma once

#include <deque>
#include <memory>
#include <stdexcept>
#include <vector>
#include <cassert>
#include <algorithm>

namespace ecs
{
    template <class T>
    class Pool
    {
    private:

        size_t m_buffer_size;
        std::deque<std::vector<std::byte>> buffers;
        std::deque<std::byte*> freeChunks; // List of free chunks within each buffer
        std::deque<std::byte*> usedChunks; // List of used chunks within each buffer

        void emplace_new_buffer()
        {
            buffers.emplace_back().resize(m_buffer_size);
            for (std::byte* p=&buffers.back()[0] ; p<&buffers.back()[buffers.back().size()-1] ; p+=sizeof(T))
            {
                freeChunks.push_back(p);
            }
        }


    public:
        Pool(size_t buffer_size) {
            m_buffer_size = buffer_size;
            // Ensure there is at least one buffer available
            emplace_new_buffer();
        }

        size_t free_chunks() {return freeChunks.size();}
        size_t used_chunks() {return usedChunks.size();}

        // Method to allocate memory for a new object and return a shared pointer
        template<typename... Args>
        std::shared_ptr<T> MakeSharedPtr(Args&&... args)
        {
            T* ptr = allocate(std::forward<Args>(args)...);
            return std::shared_ptr<T>(ptr, [this](T* ptr) {
                deallocate(ptr);
            });
        }

        // Method to allocate memory for a new object and return a unique pointer
        template<typename... Args>
        std::unique_ptr<T> MakeUniquePtr(Args&&... args)
        {
            T* ptr = allocate(std::forward<Args>(args)...);
            return std::unique_ptr<T>(ptr, [this](T* ptr) {
                deallocate(ptr);
            });
        }

        // Iterator class to iterate over used chunks
        class iterator
        {
        private:
            using ChunkIterator = typename std::deque<std::byte*>::iterator;

            ChunkIterator chunkIt;

        public:
            iterator(ChunkIterator chunkIt) : chunkIt(chunkIt) {}

            T* operator*() const {
                return reinterpret_cast<T*>((*chunkIt));
            }

            iterator& operator++() {
                ++chunkIt;
                return *this;
            }

            bool operator==(const iterator& other) const {
                return chunkIt == other.chunkIt;
            }

            bool operator!=(const iterator& other) const {
                return !(*this == other);
            }
        };

        iterator begin() {
            return iterator(usedChunks.begin());
        }

        iterator end() {
            return iterator(usedChunks.end());
        }

    private:
        // Method to allocate memory for a new object
        template<typename... Args>
        T* allocate(Args&&... args)
        {
            if (freeChunks.empty())
            {
                // No free chunks available, allocate a new buffer
                emplace_new_buffer();
            }
            for (size_t i = 0; i < buffers.size(); ++i)
            {
                if (!freeChunks.empty())
                {
                    std::byte* chunk = freeChunks.back();
                    freeChunks.pop_back();
                    usedChunks.push_back(chunk);
                    return new (chunk) T(std::forward<Args>(args)...);
                }
            }
            assert(false);
            return {};
        }

        // Method to deallocate memory for an object
        void deallocate(T* ptr)
        {
            // Find the buffer containing the memory chunk
            for (size_t i = 0; i < buffers.size(); ++i) {
                auto& buffer = buffers[i];
                size_t buffsize = m_buffer_size*sizeof(T);
                if (reinterpret_cast<std::byte*>(ptr) >= reinterpret_cast<std::byte*>(&buffer[0]) &&
                    reinterpret_cast<std::byte*>(ptr) < reinterpret_cast<std::byte*>(&buffer[buffsize]))
                {
                    // Call destructor
                    ptr->~T();
                    // Add the memory chunk to the free list of the buffer
                    freeChunks.push_back((std::byte*)ptr);

                    std::byte* ptrToRemove = (std::byte*)ptr;
                    auto it = std::find_if(usedChunks.begin(), usedChunks.end(),
                                           [ptrToRemove](std::byte* ptr) { return ptr == ptrToRemove; });
                    if (it != usedChunks.end()) {
                        usedChunks.erase(it); // Remove the pointer from the deque
                    }

                    return;
                }
            }
        }
    };
}
