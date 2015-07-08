#pragma once

#include <iro.hpp>

struct serverSettings
{
    int argc;
    const char** argv;
};

class server
{
protected:
    compositor* compositor_ = nullptr;

    serverSettings settings_;

    bool initialized_ = 0;
    bool mainLoop_ = 0;

    static server* object;

public:
    server();
    ~server();

    bool init(const serverSettings& settings);

    int run();
    void exit();

    bool isInitialized() const { return initialized_; }
    bool isInMainLoop() const { return mainLoop_; }

    compositor* getCompositor() const { return compositor_; }

    const serverSettings& getSettings() const { return settings_; }

    static server* getObject(){ return object; }
};
