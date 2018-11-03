#include <csignal>
#include <iostream>
#include <unistd.h>

bool keep_running = true;

void sigint_handler (int signum)
{
    keep_running = false;
    std::cout << "\nSignal Handled." << std::endl;
}

int main()
{
    signal(SIGINT, &sigint_handler);
    std::cout << "Starting program (press ctrl-c to stop)" << std::endl;
    while(keep_running)
    {
        std::cout << "." << std::flush;
        sleep(1);
    }
    std::cout << std::endl;
    std::cout << "Exiting program cleanly. Goodbye!" << std::endl;

    return 0;
}
