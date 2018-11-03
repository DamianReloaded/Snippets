#include <cstdio>
#include <unistd.h>
#include <string>
using namespace std;

int main()
{
    std::string tab = "";
    int id = 0;
    printf("--beginning of the program %d--\n",id);
    int counter = 0;
    pid_t pid1 = fork();
    pid_t pid2=0, pid3=0;

    if (pid1==0)
    {
        id = 1;
        tab = "  ";
        printf((tab + "--beginning of the program %d--\n").c_str(),id);
        pid2 = fork();
    }

    if (pid2>0)
    {
        id = 2;
        tab = "    ";
        printf((tab + "--beginning of the program %d--\n").c_str(),id);
        pid3 = fork();
    }

    if (pid3>0)
    {
        id = 3;
        tab = "      ";
        printf((tab + "--beginning of the program %d--\n").c_str(),id);
    }

/*    if (pid == 0)
    {
        //child process
        int i = 0;
        for (; i<5; ++i)
        {
            printf("parent process: counter=%d\n", ++counter);
        }
    }
    else
    {
        // fork failed
        printf("fork() failed! last pid: %d\n", pid);
        return 1;
    }
*/
    printf((tab + "--end of program %d--\n").c_str(),id);

    return 0;
}
