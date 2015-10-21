#pragma once

#include <iro/include.hpp>

#include <libinput.h>
#include <libudev.h>

#include <wayland-server-core.h>

class inputHandler
{
protected:
    libinput* input_ = nullptr;
    wl_event_source* inputEventSource_ = nullptr;

public:
    inputHandler();
    ~inputHandler();

    int inputEvent();
};
