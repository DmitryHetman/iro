#pragma once

#include <ostream>
#include <util/misc.hpp>

////////////
extern std::ostream* warningStream;
extern std::ostream* debugStream;
extern std::ostream* errorStream;

template<typename ... Args> void iroLog(Args&& ... args)
{
    #ifdef IRO_LOG
        printVars(*debugStream, args ...);
        *debugStream << std::endl;
    #endif // IRO_DEBUG
}

template<typename ... Args> void iroWarning(Args&& ... args)
{
    printVars(*warningStream, "warning: ", args ...);
    *warningStream << std::endl;
}


template<typename ... Args> void iroError(Args&& ... args)
{
    printVars(*errorStream, "error: ", args ...);
    *errorStream << std::endl;
}

