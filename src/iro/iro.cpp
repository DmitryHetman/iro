#include <iro/iro.hpp>
#include <iro/compositor/compositor.hpp>
#include <iro/compositor/shell.hpp>
#include <iro/backend/session.hpp>
#include <iro/backend/x11Backend.hpp>
#include <iro/backend/kmsBackend.hpp>
#include <iro/backend/renderer.hpp>
#include <iro/backend/glRenderer.hpp>
#include <iro/util/log.hpp>

#include <string>
#include <iostream>
#include <fstream>

#include <wayland-server-core.h>
#include <signal.h>
#include <unistd.h>


iro* iro::object = nullptr;
std::ofstream iroStreamLog_;

std::ostream* logStream = nullptr;
std::ostream* warningStream = &std::cout;
std::ostream* errorStream = &std::cerr;

iro* getIro()
{
    return iro::getObject();
}

sessionManager* iroSessionManager()
{
    if(!getIro()) return nullptr;
    return getIro()->getSessionManager();
}

backend* iroBackend()
{
    if(!getIro()) return nullptr;
    return getIro()->getBackend();
}

renderer* iroRenderer()
{
    if(!getIro()) return nullptr;
    return getIro()->getRenderer();
}

iroShellModule* iroShell()
{
    if(!getIro()) return nullptr;
    return getIro()->getShell();
}

eglContext* iroEglContext()
{
    if(!getIro()) return nullptr;
    return getIro()->getEGLContext();
}

unsigned int iroTime()
{
    if(!getIro()) return 0;
    return getIro()->getTime();
}

/////////////////////////////////////////////7
void signalHandler(int sig)
{
    iroError("recieved signal number ", sig, ". Exiting");
    getIro()->exit();
}

/////////////////////////////////////////////
iro::iro()
{
    timer_.reset();
}

iro::~iro()
{
    if(backend_) delete backend_;
    if(sessionManager_) delete sessionManager_;
    if(compositor_) delete compositor_;
}

bool iro::init(const iroSettings& settings)
{
    object = this;
    settings_ = settings;

    bool x11 = x11Backend::available();
    bool privileged = (geteuid() == 0);

    bool loadDevices = (!x11); //todo

    try
    {
        //init log
        if(settings_.log == "cout")
        {
            logStream = &std::cout;
        }
        else if(settings_.log != "no")
        {
            iroStreamLog_.open(settings_.log);
            if(!iroStreamLog_.is_open())
            {
                iroWarning("iro::init: could not open log file ", settings_.log, ", using std::cout");
                settings_.log = "cout";
                logStream = &std::cout;
            }
        }

        //init devices
        if(loadDevices)
        {
            sessionManager_ = new sessionManager();
            sessionManager_->initDeviceFork();
        }

        //drop permissions
        if(setuid(getuid()) < 0 || setgid(getgid()) < 0)
        {
            throw std::runtime_error("iro::init: could not drop permissions");
            return 0;
        };

        compositor_ = new compositor();

        if(!x11)
        {
            sessionManager_->initSession(!privileged); //logind only needed if not privileged
            backend_ = new kmsBackend();
        }
        else
        {
            backend_ = new x11Backend();
        }

        egl_ = new eglContext(backend_->getNativeDisplay());
        renderer_ = new glRenderer(); //todo

        for(output* o : backend_->getOutputs())
            renderer_->initOutput(*o);

        //load modules - todo
        iroModule* module = loadModule("libiro-desktop-shell.so");
        if(!module || !(shell_ = dynamic_cast<iroShellModule*>(module)))
        {
            throw std::runtime_error("iro::init: could not load desktop shell");
            return 0;
        }
    }

    catch(const std::exception& err)
    {
        iroError("iro::init failed: ", err.what());
        object = nullptr;

        return 0;
    }

    iroLog("iro::init: iro was successfully initialized! Version ", IRO_VMajor, ".", IRO_VMinor);
    initialized_ = 1;
    return 1;
}

//loader
iroModule* iro::loadModule(const std::string& modName)
{
    return nullptr; //todo
}

exitReason iro::run()
{
    if(!initialized_)
        return exitReason::initFailed;

    compositor_->run();

    return exitReason_;
}

void iro::exit(exitReason reason)
{
    exitReason_ = reason;

    if(compositor_)
        wl_display_terminate(compositor_->getWlDisplay());
}

