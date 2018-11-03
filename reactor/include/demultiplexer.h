#ifndef MULTIPLEXER_H
#define MULTIPLEXER_H

#include "queue.h"

class process;

class demultiplexer
{
    public:
                demultiplexer();
        virtual ~demultiplexer();

        void push (const message& msg)
        {
            queue[msg.type].push_front(msg);
        }

        void update();

        void subscribe(process* proc)
        {
            processes.push_back(proc);
            queue.resize(queue.size()+1);
        }

    protected:
        std::deque<std::deque<message>> queue;
        std::deque<process*> processes;

    private:
};

#endif // MULTIPLEXER_H
