#include <iostream>

using namespace std;

class base
{
    public:
        virtual int give() { return 4; }
};

class derived : public base
{
    public:
        virtual int give() { return 3; }
};

int main()
{
    base* b = new derived();

    cout << b->base::give() << endl;
    return 0;
}
