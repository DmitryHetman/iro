#include <server.hpp>
#include <compositor/compositor.hpp>
#include <util/vec.hpp>

#include <string>
#include <iostream>
#include <signal.h>


server* server::object = nullptr;

server* getServer()
{
    return server::getObject();
}

/////////////////////////////////////////////7
void signalHandler(int sig)
{
    if(getServer()) getServer()->exit();
    exit(0);
}

/////////////////////////////////////////////////////////////////////////////////////
server::server()
{
}

server::~server()
{
    exit();
}

bool server::init(const serverSettings& settings)
{
    settings_ = settings;

    if(object)
    {
        return 0;
    }

    object = this;

    //todo parse options

    try
    {
        compositor_ = new compositor();
    }
    catch(const std::exception& error)
    {
        std::cerr << error.what() << std::endl;
        delete compositor_;
        return 0;
    }

    struct sigaction action;
    action.sa_handler = &signalHandler;
    sigaction(SIGINT, &action, nullptr);
    sigaction(SIGABRT, &action, nullptr);
    sigaction(SIGHUP, &action, nullptr);
    sigaction(SIGKILL, &action, nullptr);

    initialized_ = 1;

    return 1;
}

int server::run()
{
    if(!initialized_)
    {
        return -1;
    }

    mainLoop_ = 1;
    return compositor_->run();
}

void server::exit()
{
    mainLoop_ = 0;
    if(compositor_)
    {
        delete compositor_;
        compositor_ = nullptr;
    }
}
