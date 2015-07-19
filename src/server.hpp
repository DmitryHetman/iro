#pragma once

#include <iro.hpp>

#include <util/nonCopyable.hpp>
#include <util/time.hpp>

struct serverSettings
{
    int argc;
    const char** argv;
};

class server : public nonCopyable
{
protected:
    compositor* compositor_ = nullptr;

    serverSettings settings_;

    bool initialized_ = 0;
    bool mainLoop_ = 0;

    timer timer_;


    static server* object;

public:
    server();
    ~server();

    bool init(const serverSettings& settings);

    int run();
    void exit();

    bool isInitialized() const { return initialized_; }
    bool isInMainLoop() const { return mainLoop_; }

    unsigned int getTime() const { return timer_.getElapsedTime().asMilliseconds(); }
    timeDuration getDuration() const { return timer_.getElapsedTime(); }

    compositor* iroCompositor() const { return compositor_; }

    const serverSettings& getSettings() const { return settings_; }


    static server* getObject(){ return object; }
};
