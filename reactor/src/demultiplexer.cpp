#include "demultiplexer.h"
#include "process.h"

demultiplexer::demultiplexer()
{
    //ctor
}

demultiplexer::~demultiplexer()
{
    //dtor
}

void demultiplexer::update()
{
    for (size_t k=0; k<processes.size() && !processes[k]->locked(); k++)
    {
        processes[k]->lock();
        for ( const message& msg : queue[k])
        {
            processes[k]->push(msg);
        }
        processes[k]->unlock();
        queue[k].clear();
    }
}
