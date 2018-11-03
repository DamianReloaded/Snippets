#include "demultiplexer.h"
#include "process.h"

using namespace std;

int main()
{
    demultiplexer   demu;
    process         pro0(0), pro1(1), pro2(2);
    demu.subscribe(&pro0);
    demu.subscribe(&pro1);
    demu.subscribe(&pro2);

    while (true)
    {
        for (int i=0; i<3; i++)
        {
            for (int k=0; k<=i; k++)
            {
                message msg;
                msg.type = k;
                demu.push(msg);
            }
        }
        demu.update();
        pro0.update();
        pro1.update();
        pro2.update();
    }


    return 0;
}
