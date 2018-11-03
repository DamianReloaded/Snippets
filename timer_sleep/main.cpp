#include <iostream>
#include <sys/time.h>
#include <ctime>

using namespace std;

double get_time_ms()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return (t.tv_sec*1000 + (t.tv_usec / 1000));
}

void sleep(long milliseconds)
{
    struct timespec req = { .tv_sec = milliseconds/1000, .tv_nsec = milliseconds%1000 * 1000000L };
    nanosleep(&req, NULL);
}

int main()
{
    double start = get_time_ms();
    sleep(450);
    double end = get_time_ms();
    cout << "Elapsed: " << end-start << endl;
    return 0;
}
