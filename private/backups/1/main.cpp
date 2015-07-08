#include "server.hpp"
#include <iostream>

int main(int argc, const char** argv)
{
    server ori;

    if(!ori.init())
    {
        std::cout << "failed to init" << std::endl;
        return 0;
    }

    ori.run();

    return 1;
}
