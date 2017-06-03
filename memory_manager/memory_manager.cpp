/*
OwlSL (Owl Script Language)

Copyright (c) 2013-2014 Damian Reloaded <>

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include <iostream>
#include <list>
#include <cstdlib>
#include <cstring>
#include <cassert>

namespace owl {
    class memory
    {
        public:
            struct block
            {
                block() : pos(0), size(0){}
                size_t pos;
                size_t size;
            };

            memory()
            {
                m_buffer = NULL;
            }

            void init (const size_t& _buffersize=2147483648) //4294967296
            {
                if (m_buffer!=NULL) return;

                m_bytes_allocated = 0;
                m_buffer_size = _buffersize;
                m_buffer = new char[m_buffer_size];
                memset(m_buffer, 0, m_buffer_size);

                block* b = (block*)&m_buffer[0];

                b->size = m_buffer_size-sizeof(block);
                b->pos  = 0+sizeof(block);
                m_unused.push_back(b);
            }

            void terminate()
            {
                delete [] m_buffer;
                m_buffer = NULL;
            }

            void* allocate(std::size_t size)
            {
                block* taken = 0;
                // if there is a big enough chunk at hand use it
                if (m_unused.back()->size>=size)
                {
                    taken = m_unused.back();
                    m_unused.pop_back();
                }
                else
                {
                    block* b = NULL;
                    // loop all unused chunks and find one big enough
                    for (std::list<block*>::iterator i=m_unused.begin(); i!=m_unused.end(); i++)
                    {
                        b = (*i);
                        if (b->size>=size)
                        {
                            taken = b;
                            m_unused.erase(i);
                            break;
                        }
                    }

                    assert(taken!=0);
                }

                // if the chunk is bigger than size, cut it to size and create a new free chunk with the rest
                if (taken->size>size)
                {
                    size_t newblockpos = taken->pos+size;
                    block* remaining = (block*)&m_buffer[newblockpos];
                    remaining->pos = newblockpos+sizeof(block);
                    remaining->size = taken->size - (size+sizeof(block)) ;
                    m_unused.push_back(remaining);
                }

                // allocate memory block
                taken->size = size;
                m_used.push_back(taken);
                m_bytes_allocated += size;
                return &m_buffer[taken->pos];
            }

            template <class T>
            T* allocate ()
            {
                return new (*this) T;
            }

            template <class T>
            void deallocate(T* _ptr)
            {
                _ptr->~T();
                block* b = (block*)(((void*)_ptr)-sizeof(block));
                free_block(b);
                return;
            }

            void free_block (block* _block)
            {
                block* b = NULL;
                // loop all unused blocks.
                for (std::list<block*>::iterator i=m_unused.begin(); i!=m_unused.end(); i++)
                {
                    b = (*i);
                    // if unused is contiguous
                    if ( (b->pos+b->size)==(_block->pos-sizeof(block)) )
                    {
                        b->size += _block->size+sizeof(block);
                        m_used.remove(_block);
                        return;
                    }
                    else if ( (_block->pos+_block->size)==(b->pos-sizeof(block)) )
                    {
                        _block->size += b->size+sizeof(block);
                        m_used.remove(_block);
                        m_unused.remove(b);
                        m_unused.push_back(_block);
                        return;
                    }
                }

                // insert this block as a free block.
                m_used.remove(_block);
                m_unused.push_back(_block);
            }

            void defrag()
            {
                bool running = true;
                bool restart = false;
                block* b = NULL;
                block* b2 = NULL;
                // loop all unused blocks.

                while (running)
                {
                    restart = false;
                    for (std::list<block*>::iterator k=m_unused.begin(); k!=m_unused.end()&&!restart; k++)
                    {
                        b2 = (*k);
                        for (std::list<block*>::iterator i=m_unused.begin(); i!=m_unused.end()&&!restart; i++)
                        {
                            b = (*i);
                            // if unused is contiguous
                            if ( (b->pos+b->size)==(b2->pos-sizeof(block)) )
                            {
                                b->size += b2->size+sizeof(block);
                                m_unused.remove(b2);
                                restart = true;
                                break;
                            }
                            else if ( (b2->pos+b2->size)==(b->pos-sizeof(block)) )
                            {
                                b2->size += b->size+sizeof(block);
                                m_unused.remove(b2);
                                m_unused.remove(b);
                                m_unused.push_back(b2);
                                restart = true;
                                break;
                            }
                        }
                    }
                    if (!restart) running = false;
                }
            }

            std::list<block*>   m_used;
            std::list<block*>   m_unused;

            size_t              m_bytes_allocated;
            size_t              m_buffer_size;
            char*               m_buffer;
    };
}

void * operator new (std::size_t size, owl::memory& mem)
{
    return mem.allocate(size) ;
}
