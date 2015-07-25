#include <server.hpp>
#include <compositor/compositor.hpp>
#include <backend/session.hpp>

#include <backend/x11/x11Backend.hpp>
#include <backend/kms/kmsBackend.hpp>

#include <log.hpp>

#include <string>
#include <iostream>
#include <ostream>
#include <fstream>

#include <wayland-server-core.h>
#include <signal.h>
#include <unistd.h>


server* server::object = nullptr;
std::ofstream iroStreamLog_;

std::ostream* logStream = nullptr;
std::ostream* warningStream = &std::cout;
std::ostream* errorStream = &std::cerr;

server* iroServer()
{
    return server::getObject();
}

sessionManager* iroSessionManager()
{
    if(!iroServer()) return nullptr;
    return iroServer()->getSessionManager();
}

backend* iroBackend()
{
    if(!iroServer()) return nullptr;
    return iroServer()->getBackend();
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

    if(compositor_) delete compositor_;
    if(sessionManager_) delete sessionManager_;
    if(backend_) delete backend_;
}

bool server::init(const serverSettings& settings)
{
    bool onX11 = x11Backend::available();
    bool privileged = 0;

    if(!onX11)
    {
        if(geteuid() == 0)
        {
            //root
            privileged = 1;
        }

        try
        {
            sessionManager_ = new sessionManager();
        }
        catch(const std::exception& err)
        {
            iroError(err.what());
            return 0;
        }
    }


    if(settings.log.empty() || settings.log == "no")
    {
        logStream = nullptr;
    }
    else if(settings.log == "cout")
    {
        logStream = &std::cout;
    }
    else
    {
        iroStreamLog_.open(settings.log);
        if(!iroStreamLog_.is_open()) return 0;
        logStream = &iroStreamLog_;
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
        if(x11Backend::available())
        {
            backend_ = new x11Backend();
        }
        else
        {
            backend_ = new kmsBackend();
        }
    }
    catch(const std::exception& error)
    {
        iroError(error.what());
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
