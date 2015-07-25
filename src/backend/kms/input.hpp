#pragma once

#include <iro.hpp>

#include <libinput.h>
#include <libudev.h>

#include <wayland-server-core.h>

class inputHandler
{
protected:
    friend int inputEventLoop(int fd, unsigned int mask, void* data);
    friend int udevEventLoop(int fd, unsigned int mask, void* data);

    libinput* input_ = nullptr;
    udev* udev_ = nullptr;

    udev_monitor* udevMonitor_ = nullptr;

    wl_event_source* inputEventSource_ = nullptr;
    wl_event_source* udevEventSource_ = nullptr;

    int inputEvent();
    int udevEvent();

public:
    inputHandler(sessionManager& h);
    ~inputHandler();
};
