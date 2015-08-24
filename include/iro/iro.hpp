#pragma once

#include <iro/include.hpp>
#include <iro/util/iroModule.hpp>

#include <nyutil/nonCopyable.hpp>
#include <nyutil/time.hpp>

#include <string>
#include <vector>

//settings
struct iroSettings
{
    std::string log = "iro.log";
    bool login = 0;
};

//exitReason
enum class exitReason : unsigned char
{
    none = 0,

    passedArguments = 1,
    initFailed = 2,
    runtimeError = 3,
    noOutputs = 4,
    receivedSignal = 5
};

//iro
class iro : public nonCopyable, public moduleLoader
{
protected:
    std::vector<iroModule*> modules_;
    std::vector<output*> outputs_;

    compositor* compositor_ = nullptr; //deals with all the wayland stuff
<<<<<<< HEAD
    sessionManager* sessionManager_ = nullptr; //deals with logind/dbus
    backend* backend_ = nullptr; //deals with displaying the outputs, input
    eglContext* egl_ = nullptr; //connects wayland and compositor
    renderer* renderer_ = nullptr; //renders on the outputs
=======
    backend* backend_ = nullptr;
    eglContext* egl_ = nullptr;
    renderer* renderer_ = nullptr;
    sessionManager* sessionManager_ = nullptr; //deals with logind/dbus
    iroShellModule* shell_ = nullptr; //displays some interface and all clients
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527

    iroSettings settings_;
    timer timer_;

    exitReason exitReason_ = exitReason::none;

    static iro* object;

public:
    iro();
    virtual ~iro();

    bool init(const iroSettings& settings);

    exitReason run();
    void exit(exitReason reason = exitReason::none);

    unsigned int getTime() const { return timer_.getElapsedTime().asMilliseconds(); }
    timeDuration getDuration() const { return timer_.getElapsedTime(); }

    compositor* getCompositor() const { return compositor_; }
    backend* getBackend() const { return backend_; }
    renderer* getRenderer() const { return renderer_; }
    sessionManager* getSessionManager() const { return sessionManager_; }

    const iroSettings& getSettings() const { return settings_; }
    virtual iroModule* loadModule(const std::string& modName) override; //module

    static iro* getObject(){ return object; }
};
