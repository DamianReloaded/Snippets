#include "process.h"

process::process(int id) : m_id(id)
{
    m_locked = false;
}

process::~process()
{
    //dtor
}
