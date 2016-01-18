#pragma once

#include <iro/include.hpp>
#include <nytl/callback.hpp>
#include <nytl/nonCopyable.hpp>

#include <string>

//prototypes
typedef struct DBusMessage DBusMessage;

namespace iro
{

///The Device class represents a socket to a phyical or virtual device.
///It is created an managed by a DeviceManager object.
class Device : public nytl::nonCopyable
{
protected:
	friend class DeviceHandler;

	nytl::callback<void(const Device&)> pauseCallback_;
	nytl::callback<void(const Device&)> resumeCallback_;

    std::string path_;
    int fd_;
    bool active_;
	DeviceHandler* handler_;

public:
	//Both functions are preserved to deviceHandler
	Device() = default;
	~Device() = default;

	///Releases the device which closes the file desciptor and implicitly destroys the object.
    void release();

	///Returns the path this device was created from.
	const std::string& path() const { return path_; }

	///Returns the fd of this device.
	int fd() const { return fd_; }

	///Returns whether the device is still active.
	bool active() const { return active_; }

	///Registers a callback for a function that should be called when the device gets paused.
	///The signature of the given function must be compatible to void(const Device&)
    template<typename F> nytl::connection onPause(F&& fnc){ return pauseCallback_.add(fnc); }

	///Registers a callback for a function that should be called when the device gets resumed.
	///The signature of the given function must be compatible to void(const Device&)
    template<typename F> nytl::connection onResume(F&& fnc){ return resumeCallback_.add(fnc); }
};

//TODO: it does make more sense to write devicehandler as an interface and provide implementations
//for it like e.g. fork, normal and dbus

///The DeviceHandler class manages devices. It can either make use of the root rights of the
///application by creating a fork and then using it for device creation/destruction or
///it can open/close devices over the logind/dbus interface.
class DeviceHandler : public nytl::nonCopyable
{
protected:
	std::vector<std::unique_ptr<Device>> devices_;
	int forkSocket_ = 0;
	DBusHandler* dbus_ = nullptr;
	LogindHandler* logind_ = nullptr;

	std::string sessionPath() const;

	void dbusPauseDevice(DBusMessage* msg);
	void dbusResumeDevice(DBusMessage* msg);

	Device* takeDeviceDBus(const std::string& path, int flags);
	Device* takeDeviceNormal(const std::string& path, int flags);

	bool releaseDeviceDBus(int fd);
	bool releaseDeviceNormal(int fd);

public:
	///Constructs the DeviceHandler and creates an application fork with root rights (if the 
	///application has them at the moment). If the useRoot parameter is not set, it 
	///will just try to manage device with normal fds. 
	DeviceHandler(bool useRoot = 1);

	///Constructs the DeviceHandler which will use the logind/dbus backend to manage its
	///devices. 
	DeviceHandler(DBusHandler& dbus, LogindHandler& logind);
	~DeviceHandler();

	//TODO: make it return a reference and throw on error
	///Tries to create a device for path. Returns nullptr on failure.
	Device* takeDevice(const std::string& path, int flags = 0);

	///Returns the Device for a given filedescriptor, nullptr if none is found.
	Device* deviceForFd(int fd) const;

	///Closes and implicitly deletes a device. If the device is not registered in this
	///deviceHandler, it returns nullptr.
	bool releaseDevice(const Device& dev);

	///Returns the used dbus handler if the DeviceHandler uses the dbus backend, nullptr otherwise.
	DBusHandler* dbusHandler() const { return dbus_; }

	///Returns the logind handler if the DeviceHandler uses the dbus backend, nullptr otherwise.
	LogindHandler* logindHandler() const { return logind_; }
};

}
