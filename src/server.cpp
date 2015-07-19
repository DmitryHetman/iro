#include <server.hpp>
#include <compositor/compositor.hpp>

#include <log.hpp>

#include <string>
#include <iostream>
#include <ostream>
#include <fstream>

#include <wayland-server-core.h>
#include <signal.h>


server* server::object = nullptr;
std::ofstream iroStreamLog_;

std::ostream* debugStream = &iroStreamLog_;
//std::ostream* debugStream = &std::cout;
std::ostream* warningStream = &std::cout;
std::ostream* errorStream = &std::cerr;

server* iroServer()
{
    return server::getObject();
}

unsigned int iroTime()
{
    if(!iroServer()) return 0;
    return iroServer()->getTime();
}

/////////////////////////////////////////////7
void signalHandler(int sig)
{
    iroError("recieved signal number ", sig, ". Exiting");
    iroServer()->exit();
}

/////////////////////////////////////////////////////////////////////////////////////
server::server()
{
    timer_.reset();
}

server::~server()
{
    if(mainLoop_)exit();
    if(compositor_)delete compositor_;
}

bool server::init(const serverSettings& settings)
{
    iroStreamLog_.open("log");
    if(!iroStreamLog_.is_open())
    {
        return 0;
    }

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
        wl_display_terminate(compositor_->getWlDisplay());
    }
}
