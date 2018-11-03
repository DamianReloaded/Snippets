#ifndef PROCESS_H
#define PROCESS_H
#include <assert.h>
#include "queue.h"

class process
{
    public:
                    process(int id);
        virtual     ~process();

        void lock() {m_locked = true;}
        void unlock() {m_locked = false;}
        const bool& locked() { return m_locked; }

        void push(const message& msg)
        {
            assert(m_locked && "Process mutex not locked.");
            queue.push_front(msg);
        }

        void update()
        {
            if (queue.size()<1 || m_locked) return;

            lock();

            for (message& msg : queue)
            {
                std::cout << msg.type <<std::endl;
            }
            queue.clear();

            unlock();
        }

    protected:
        std::deque<message> queue;
        int m_id;
        bool m_locked;
    private:
};

#endif // PROCESS_H
