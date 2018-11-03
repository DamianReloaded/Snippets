#include <iostream>
#include <cstdlib>

using namespace std;

int main()
{
    char* home_path (getenv("HOME"));
    if (home_path != NULL)
    cout << home_path << endl;
    return 0;
}
