#include <iro/include.hpp>
#include <iro/iro.hpp>

#include <nyutil/argParser.hpp>

#include <iostream>
#include <vector>
#include <string>

int main(int argc, const char** argv)
{
    iroSettings settings;

    //parse args
    {
        argParser parser;

        parser.addStringVar("log", settings.log, "set the log type {<filename>; cout; no}", "core", "");
        parser.addFlag("login", settings.login, "show login screen", "core", "");

        unsigned char ret = parser.parse(argc, argv);
        if(ret & argParser::helpCalled || ret & argParser::malformedToken)
            return -1;
    }

    //init
    iro ori;
    if(!ori.init(settings))
    {
        std::cout << "failed to init iro" << std::endl;
        return -2;
    }

    //run
    int ret = ori.run();
    std::cout << "exiting" << std::endl;

    return ret;
}
