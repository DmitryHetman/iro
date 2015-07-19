#pragma once

#include <iro.hpp>

#include <dbus/dbus.h>
#include <string>

class sessionHandler
{
protected:
    std::string seat_;
    std::string session_;

    DBusConnection* dbus_ = nullptr;
    wl_event_source* dbusEventSource_ = nullptr;

    unsigned int vt_ = 0;

public:
    sessionHandler();
    ~sessionHandler();


};
