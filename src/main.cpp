#include <iro.hpp>
#include <server.hpp>

#include <util/argParser.hpp>

#include <iostream>
#include <vector>
#include <string>


int main(int argc, const char** argv)
{
    server ori;

    serverSettings settings;

    {
        argParser parser;

        parser.addStringVar("log", settings.log, "set the log type {<filename>; cout; no}", "core", "");

        unsigned char ret = parser.parse(argc, argv);
        if(ret & argParser::helpCalled || ret & argParser::malformedToken)
            return -1;
    }


    if(!ori.init(settings))
    {
        std::cout << "failed to init iro" << std::endl;
        return -2;
    }

    return ori.run();
}
