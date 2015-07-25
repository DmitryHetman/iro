#pragma once

#include <ostream>
#include <util/misc.hpp>

////////////
extern std::ostream* logStream;
extern std::ostream* warningStream;
extern std::ostream* errorStream;

template<typename ... Args> void iroLog(Args&& ... args)
{
    #ifdef IRO_LOG
        if(!logStream) return;

        printVars(*logStream, args ...);
        *logStream << std::endl;
    #endif // IRO_DEBUG
}

template<typename ... Args> void iroWarning(Args&& ... args)
{
    iroLog("warning: ", args ...);

    printVars(*warningStream, "warning: ", args ...);
    *warningStream << std::endl;
}


template<typename ... Args> void iroError(Args&& ... args)
{
    iroLog("error: ", args ...);

    printVars(*errorStream, "error: ", args ...);
    *errorStream << std::endl;
}

