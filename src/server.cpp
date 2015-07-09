#include <server.hpp>
#include <compositor/compositor.hpp>
#include <util/vec.hpp>

#include <string>
#include <iostream>

#include <ostream>
#include <fstream>

#include <signal.h>


server* server::object = nullptr;
std::ofstream iroStreamLog_;
std::ostream& iroLog = iroStreamLog_;

server* getServer()
{
    return server::getObject();
}

/////////////////////////////////////////////7
void signalHandler(int sig)
{
    if(getServer()) getServer()->exit();
}

/////////////////////////////////////////////////////////////////////////////////////
server::server()
{
}

server::~server()
{
    if(mainLoop_)exit();

    delete compositor_;
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
