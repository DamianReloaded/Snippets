#include <fstream>
#include <iostream>
#include <string>

int main(const int argc, const char ** argv)
{
    std::string filename = "myfile.txt";
    std::ifstream file;
    file.open(filename.c_str());
    if (!file.is_open())
    {
        std::cerr << "Error opening file:" << filename << ". Aborting program." <<std::endl;
        return 1;
    }
    int line_count = 0;
    std::string line;
    while (file.good())
    {
        std::getline(file,line);
        std::cout << ++line_count << ": " << line << std::endl;
    }
    file.close();

    return 0;
}
