#pragma once

#include <iro/include.hpp>
#include <nytl/callback.hpp>
#include <nytl/nonCopyable.hpp>

#include <string>

//prototypes
typedef struct DBusMessage DBusMessage;
typedef struct DBusMessageIter DBusMessageIter;
typedef struct DBusPendingCall DBusPendingCall;

namespace iro
{

///The Device class represents a socket to a phyical or virtual device.
///It is created an managed by a DeviceManager object.
class Device
{
	friend class DeviceHandler;
protected:
	nytl::callback<void(const Device&)> pauseCallback_;
	nytl::callback<void(const Device&)> resumeCallback_;

    std::string path_;
    int fd_;
    bool active_;
	DeviceHandler* handler_;

public:
    void release();

	const std::string& path() const { return path_; }
	int fd() const { return fd_; }
	bool active() const { return active_; }

    template<typename F> nytl::connection onPause(F&& fnc){ return pauseCallback_.add(fnc); }
    template<typename F> nytl::connection onResume(F&& fnc){ return resumeCallback_.add(fnc); }
};

///The DeviceHandler class manages devices. It can either make use of the root rights of the
///application by creating a fork and then using it for device creation/destruction or
///it can open/close devices over the logind/dbus interface.
class DeviceHandler
{
protected:
	std::vector<std::unique_ptr<Device>> devices_;
	int forkSocket_ = 0;
	DBusHandler* dbus_ = nullptr;
	LogindHandler* logind_ = nullptr;

	DBusPendingCall* pendingActive_ = nullptr;

	std::string sessionPath() const;

	void dbusPauseDevice(DBusMessage* msg);
	void dbusResumeDevice(DBusMessage* msg);
	void dbusPropertiesChanged(DBusMessage* msg);

	void dbusGetActive();
	void dbusParseActive(DBusMessage* m, DBusMessageIter* it);
	static void dbusGetActiveCallback(DBusPendingCall* call, void* data);

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
