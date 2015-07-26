#pragma once

#include <iro/include.hpp>
#include <iro/iroModule.hpp>

#include <nyutil/nonCopyable.hpp>
#include <nyutil/time.hpp>

#include <string>
#include <vector>


struct iroSettings
{
    std::string log = "iro.log";
    bool login = 0;
};

class iro : public nonCopyable, public moduleLoader
{
private:
    //setup util
    void setupCompositor();
    void setupSession(bool onX11, bool privileged);
    void setupBackend(bool onX11);
    void loadModules(bool loginShell);

protected:
    std::vector<iroModule*> modules_;

    compositor* compositor_ = nullptr;
    sessionManager* sessionManager_ = nullptr;
    backend* backend_ = nullptr;

    iroSettings settings_;

    bool initialized_ = 0;
    bool mainLoop_ = 0;

    timer timer_;

    iroShellModule* loadShellModule(const std::string& modName);


    static iro* object;

public:
    iro();
    ~iro();

    bool init(const iroSettings& settings);

    int run();
    void exit();

    bool isInitialized() const { return initialized_; }
    bool isInMainLoop() const { return mainLoop_; }

    unsigned int getTime() const { return timer_.getElapsedTime().asMilliseconds(); }
    timeDuration getDuration() const { return timer_.getElapsedTime(); }

    compositor* getCompositor() const { return compositor_; }
    sessionManager* getSessionManager() const { return sessionManager_; }
    backend* getBackend() const { return backend_; }

    const iroSettings& getSettings() const { return settings_; }

    //module
    iroModule* loadModule(const std::string& modName) override;


    static iro* getObject(){ return object; }
};
