#pragma once

#include <iro/include.hpp>
#include <nytl/callback.hpp>
#include <nytl/nonCopyable.hpp>

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

public:
	///Constructs the DeviceHandler and creates an application fork with root rights (if the 
	///application has them at the moment). If the useRoot parameter is not set, it 
	///will just try to manage device with normal fds. 
	DeviceHandler(bool useRoot = 1);

	///Constructs the DeviceHandler which will use the logind/dbus backend to manage its
	///devices. It uses the compositors event loop to catch pause/resume events.
	DeviceHandler(Compositor& comp, LogindHandler& logind);

	~DeviceHandler();

	///Tries to create a device for path. Returns nullptr on failure.
	Device* createDevice(const std::string& path, int flags = 0);

	///Closes and implicitly deletes a device. If the device is not registered in this
	///deviceHandler, it returns nullptr.
	bool destroyDevice(const Device& dev);
};

}
