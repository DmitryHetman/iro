#pragma once

#include <iro.hpp>

#include <util/callback.hpp>

#include <dbus/dbus.h>
#include <security/pam_appl.h>

#include <string>

//device
class device
{
friend class sessionHandler;

protected:
    callback<void(const device&)> pauseCallback_;
    callback<void(const device&)> resumeCallback_;

public:
    void release();

    std::string path;
    int fd;
    bool active;

    connection& onPause(std::function<void(const device&)> fnc){ return pauseCallback_.add(fnc); }
    connection& onResume(std::function<void(const device&)> fnc){ return resumeCallback_.add(fnc); }

    connection& onPause(std::function<void()> fnc){ return pauseCallback_.add([=](const device&){fnc();}); }
    connection& onResume(std::function<void()> fnc){ return resumeCallback_.add([=](const device&){fnc();}); }
};

//sessionHandler
class sessionHandler
{
friend void cbDevicePaused(DBusMessage*);
friend void cbDeviceResumed(DBusMessage*);

protected:
    std::string seat_;
    std::string session_;
    std::string sessionPath_;

    DBusConnection* dbus_ = nullptr;
    wl_event_source* dbusEventSource_ = nullptr;

    unsigned int vt_ = 0;

    std::vector<device*> devices_; //all taken devices

    //cbs
    void devicePaused(DBusMessage* msg);
    void deviceResumed(DBusMessage* msg);

public:
    sessionHandler();
    ~sessionHandler();

    DBusConnection* getDBusConnection() const { return dbus_; }
    unsigned int getVTNumber() const { return vt_; }

    std::string getSeat() const { return seat_; }
    std::string getSession() const { return session_; }
    std::string getSessionPath() const { return sessionPath_; }

    device* takeDevice(const std::string& path);
    void releaseDevice(device& dev);
    void releaseDevice(int devFD);
};
