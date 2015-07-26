#include <iro/iro.hpp>
#include <iro/compositor/compositor.hpp>
#include <iro/shell/shell.hpp>
#include <iro/backend/session.hpp>
#include <iro/backend/x11/x11Backend.hpp>
#include <iro/backend/kms/kmsBackend.hpp>
#include <iro/log.hpp>

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
    if(mainLoop_) exit();

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

    try
    {
        if(settings_.log == "cout")
        {
            logStream = &std::cout;
        }
        else if(settings_.log != "no")
        {
            iroStreamLog_.open(settings_.log);
            if(!iroStreamLog_.is_open())
            {
                iroWarning("could not open log file ", settings_.log, ", using std::cout");
                settings_.log = "cout";
                logStream = &std::cout;
            }
        }

        setupCompositor();
        setupSession(x11, privileged);
        setupBackend(x11);
        loadModules(settings_.login);
    }

    catch(const std::exception& err)
    {
        iroError("iro::init failed: ", err.what());
        object = nullptr;

        return 0;
    }

    iroLog("iro was successfully initialized. Version ", IRO_VMajor, ".", IRO_VMinor);
    initialized_ = 1;
    return 1;
}

void iro::setupCompositor()
{
    compositor_ = new compositor();
}

void iro::setupSession(bool onX11, bool privileged)
{
    bool uselogind = (!onX11 && !privileged);
    sessionManager_ = new sessionManager(uselogind);
}

void iro::setupBackend(bool onX11)
{
    if(onX11)
    {
        backend_ = new x11Backend();
    }
    else
    {
        backend_ = new kmsBackend();
    }
}

void iro::loadModules(bool loginShell)
{
    if(loginShell)
    {
        iroShellModule* mod = loadShellModule("libiro-login-shell.so");
        if(!mod)
        {
            throw std::runtime_error("iro::loadModules: could not load login shell");
            return;
        }

        compositor_->getShell()->setModule(*mod);
    }
    else
    {
        iroShellModule* mod = loadShellModule("libiro-desktop-shell.so");
        if(!mod)
        {
            throw std::runtime_error("iro::loadModules: could not load desktop shell");
            return;
        }

        compositor_->getShell()->setModule(*mod);
    }


    //load listed modules
    std::string home = getenv("HOME");

    std::ifstream modStream;
    modStream.open(home + "/.config/iro/modules");

    if(!modStream.is_open())
    {
        iroWarning("iro::loadModules: could not open ~/config/iro/modules. Failed to load modules");
        return;
    }

    while(!modStream.eof())
    {
        std::string tmp;
        if(!std::getline(modStream, tmp, '\n'))
            return;

        iroModule* mod = loadModule(tmp);
        if(mod)
        {
            iroLog("iro::loadModules: loaded module ", tmp);
            modules_.push_back(mod);
        }
        else
        {
            iroWarning("iro::loadModules: failed to load module ", tmp);
        }
    }
}

//loader
iroModule* iro::loadModule(const std::string& modName)
{
    module* mod = moduleLoader::loadModule(modName);
    if(mod)
    {
        iroModule* ret = dynamic_cast<iroModule*>(mod);
        if(!ret)
        {
            moduleLoader::unloadModule(*mod);
            return nullptr;
        }
        return ret;
    }

    return nullptr;
}

iroShellModule* iro::loadShellModule(const std::string& modName)
{
        iroModule* mod = loadModule(modName);
        if(!mod) return nullptr;

        iroShellModule* shellMod = dynamic_cast<iroShellModule*>(mod);
        if(!shellMod) unloadModule(*mod);

        return shellMod;
}

int iro::run()
{
    if(!initialized_)
        return -1;

    mainLoop_ = 1;

    return compositor_->run();
}

void iro::exit()
{
    mainLoop_ = 0;

    if(compositor_)
        wl_display_terminate(compositor_->getWlDisplay());
}

