#pragma once

#include <iostream>
#include <nyutil/misc.hpp>

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
    printVars(*warningStream, "warning: ", args ...);
    *warningStream << std::endl;

    if(warningStream != logStream && logStream != &std::cout) iroLog("warning: ", args ...);
}


template<typename ... Args> void iroError(Args&& ... args)
{
    printVars(*errorStream, "error: ", args ...);
    *errorStream << std::endl;

    if(errorStream != logStream && logStream != &std::cout) iroLog("error: ", args ...);
}

