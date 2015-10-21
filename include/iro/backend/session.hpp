#pragma once

#include <iro/include.hpp>

#include <nyutil/callback.hpp>
#include <nyutil/nonCopyable.hpp>

#include <dbus/dbus.h>
#include <libudev.h>
#include <string>

//device
class device
{
friend class sessionManager;

protected:
    callback<void(const device&)> pauseCallback_;
    callback<void(const device&)> resumeCallback_;

public:
    void release();

    std::string path;
    int fd;
    bool active;

    std::unique_ptr<connection> onPause(std::function<void(const device&)> fnc){ return pauseCallback_.add(fnc); }
    std::unique_ptr<connection> onResume(std::function<void(const device&)> fnc){ return resumeCallback_.add(fnc); }

    std::unique_ptr<connection> onPause(std::function<void()> fnc){ return pauseCallback_.add([=](const device&){fnc();}); }
    std::unique_ptr<connection> onResume(std::function<void()> fnc){ return resumeCallback_.add([=](const device&){fnc();}); }
};


//sessionHandler
class sessionManager : public nonCopyable
{
protected:
    //cbs
    friend void ttySignalhandler(int);
    friend DBusHandlerResult dbusFilter(DBusConnection*, DBusMessage*, void*);
    friend int udevEventLoop(int, unsigned int, void*);
    friend void cbDevicePaused(DBusMessage*);
    friend void cbDeviceResumed(DBusMessage*);

    void enteredTTY();
    void leftTTY();
    void devicePaused(DBusMessage* msg);
    void deviceResumed(DBusMessage* msg);
    int udevEvent();

    //util setup
    void logindInit();
    void ttyInit();
    void udevInit();

    //session/tty
    std::string seat_;
    std::string session_;
    std::string sessionPath_;

    DBusConnection* dbus_ = nullptr;
    wl_event_source* dbusEventSource_ = nullptr;

    unsigned int originalVT_ = 0;
    unsigned int usedVT_ = 0;

    device* vt_ = nullptr;
    bool vtActive_ = 0;

    //devices
    udev* udev_ = nullptr;
    udev_monitor* udevMonitor_ = nullptr;
    wl_event_source* udevEventSource_ = nullptr;

    pid_t child_ = 0;
    std::vector<device*> devices_; //all taken devices

    //callbacks
    callback<void()> beforeEnter_;
    callback<void()> beforeLeave_;
    callback<void()> afterEnter_;
    callback<void()> afterLeave_;
    callback<void(bool, int)> deviceHotpluggedCallback_;

public:
    sessionManager();
    ~sessionManager();

    void initDeviceFork(); //should be called as early as possible (fork)
    void initSession(bool logind); //can only be called if compositor is initialized, not needed on x11

    //session
    DBusConnection* getDBusConnection() const { return dbus_; }

    unsigned int getVTNumber() const { return usedVT_; }
    std::string getSeat() const { return seat_; }
    std::string getSession() const { return session_; }

    bool active() const { return vtActive_; }

    //devices
    udev* getUDev() const { return udev_; }

    device* takeDevice(const std::string& path);
    void releaseDevice(device& dev);
    void releaseDevice(int devFD);

    //callbacks
    std::unique_ptr<connection> beforeEnterTTY(std::function<void()> f){ return beforeEnter_.add(f); }
    std::unique_ptr<connection> beforeLeaveTTY(std::function<void()> f){ return beforeLeave_.add(f); }
    std::unique_ptr<connection> afterEnterTTY(std::function<void()> f){ return afterEnter_.add(f); }
    std::unique_ptr<connection> afterLeaveTTY(std::function<void()> f){ return afterLeave_.add(f); }
    std::unique_ptr<connection> onDeviceHotplug(std::function<void(bool added, int fd)> f){ return deviceHotpluggedCallback_.add(f); }
};
