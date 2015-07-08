#include "server.hpp"
#include <iostream>

int main(int argc, const char** argv)
{
    server ori;

    serverSettings settings;
    settings.argc = argc;
    settings.argv = argv;

    if(!ori.init(settings))
    {
        std::cout << "failed to init" << std::endl;
        return 0;
    }

    return ori.run();
}
