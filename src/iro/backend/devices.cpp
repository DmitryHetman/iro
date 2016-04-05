#include <iro/backend/devices.hpp>
#include <iro/backend/dbus.hpp>
#include <iro/backend/logind.hpp>
#include <iro/compositor/compositor.hpp>

#include <ny/base/log.hpp>

#include <dbus/dbus.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

namespace iro
{

//Device
void Device::release()
{
	handler_->releaseDevice(*this);
}

//DeviceHandler
DeviceHandler::DeviceHandler(bool useRoot)
{
	//todo: fork communication
	if(useRoot)
	{
		if(geteuid() != 0)
		{
			ny::sendWarning("DeviceHandler::DeviceHandler: geteuid != 0");
			return;
		}
	
		int sockpair[2];
		if (socketpair(AF_LOCAL, SOCK_SEQPACKET | SOCK_CLOEXEC, 0, sockpair) != 0)
		{
			ny::sendWarning("DeviceHandler::DeviceHandler: socketpair failed");
			return;
		}

		pid_t child = fork();
		if(child < 0)
		{
			ny::sendWarning("DeviceHandler::DeviceHandler: fork failed");
			return;
		}
		else if(child == 0)
		{
			close(sockpair[0]);
			forkSocket_ = sockpair[1];

			//ny::sendLog("DeviceHandler: fork exiting");
			_exit(EXIT_SUCCESS);
		}
		else
		{
			close(sockpair[1]);
			forkSocket_ = sockpair[0];
		}	
	}
}

DeviceHandler::DeviceHandler(DBusHandler& dbus, LogindHandler& logind) 
	: dbus_(&dbus), logind_(&logind)
{
	const std::string path("org.freedesktop.login1");

    dbus.addSignalMatch(path, "org.freedesktop.login1.Session", "PauseDevice", 
		logind.sessionPath(), nytl::memberCallback(&DeviceHandler::dbusPauseDevice, this));
    dbus.addSignalMatch(path, "org.freedesktop.login1.Session", "ResumeDevice",
		logind.sessionPath(), nytl::memberCallback(&DeviceHandler::dbusResumeDevice, this));
}

DeviceHandler::~DeviceHandler()
{
	//todo: release? dbus?
}

Device* DeviceHandler::takeDevice(const std::string& path, int flags)
{
	if(dbus_)
	{
		return takeDeviceDBus(path, flags);
	}
	else
	{
		return takeDeviceNormal(path, flags);
	}

	//todo: fork communication
}

bool DeviceHandler::releaseDevice(const Device& dev)
{
	int fd = 0;
	for(auto it = devices_.cbegin(); it != devices_.cend(); ++it)
	{
		if((*it).get() == &dev)
		{
			fd = dev.fd();
			devices_.erase(it);
			break;
		}
	}

	if(!fd)
	{
		ny::sendWarning("DeviceHandler::releaseDevice: device not found.");
		return 0;
	}

	if(dbus_)
	{
		return releaseDeviceDBus(fd);
	}
	else
	{
		return releaseDeviceNormal(fd);
	}

	//todo: fork communication
}

Device* DeviceHandler::takeDeviceDBus(const std::string& path, int flags)
{
	int fd = 0;
	struct stat st;
    if(stat(path.c_str(), &st) < 0 || !S_ISCHR(st.st_mode))
    {
		ny::sendWarning("DeviceHandler::takeDeviceDBus: failed to get stat struct for path");
        return nullptr;
    }

    unsigned int majr = major(st.st_rdev);
    unsigned int minr = minor(st.st_rdev);

    DBusMessage* msg;
	DBusError error;
	dbus_error_init(&error);
	std::string errStr = "no dbus error available";

    if(!(msg = dbus_message_new_method_call("org.freedesktop.login1", sessionPath().c_str(), 
		"org.freedesktop.login1.Session", "TakeDevice")))
    {
		ny::sendWarning("DeviceHandler::takeDeviceDBus: dbus_message_new_method_call failed");
        dbus_message_unref(msg);
        return nullptr;
    }

    if(!dbus_message_append_args(msg, DBUS_TYPE_UINT32, &majr, DBUS_TYPE_UINT32, &minr, 
		DBUS_TYPE_INVALID))
    {
		ny::sendWarning("DH::takeDeviceDBus: dbus_message_append_args failed");
        dbus_message_unref(msg);
        return nullptr;
    }

    DBusMessage *reply;
    if(!(reply = dbus_connection_send_with_reply_and_block(&dbusHandler()->dbusConnection(), msg, 
		-1, &error)))
    {
		DBusHandler::checkError(error, errStr);
		ny::sendWarning("DH::takeDevDBus: dbus_connection_send_with_reply_and_block: ", errStr);
        dbus_message_unref(msg);
        return nullptr;
    }

    dbus_bool_t paused;
    if(!dbus_message_get_args(reply, nullptr, DBUS_TYPE_UNIX_FD, &fd, DBUS_TYPE_BOOLEAN, 
		&paused, DBUS_TYPE_INVALID))
    {
		ny::sendWarning("DH::takeDeviceDBus: dbus_message_get_args failed");
        dbus_message_unref(reply);
        dbus_message_unref(msg);
        return nullptr;
    }

    int fl;
    if((fl = fcntl(fd, F_GETFL)) < 0 || fcntl(fd, F_SETFD, fl | FD_CLOEXEC) < 0)
    {
		ny::sendWarning("DH::takeDeviceDBus: invalid fd");
        close(fd);
        dbus_message_unref(reply);
        dbus_message_unref(msg);
        return nullptr;
    }

    dbus_message_unref(reply);
    dbus_message_unref(msg);

	auto dev = std::make_unique<Device>();
	dev->fd_ = fd;
	dev->active_ = !paused;
	dev->handler_ = this;
	dev->path_ = path;

	auto* ret = dev.get();
	devices_.push_back(std::move(dev));

	return ret;
}

Device* DeviceHandler::takeDeviceNormal(const std::string& path, int flags)
{
	int fd = open(path.c_str(), flags | O_CLOEXEC);
	if(fd < 0)
	{
		ny::sendWarning("DeviceHandler::takeDeviceNormal: open failed with code ", fd);
		return nullptr;
	}

	auto dev = std::make_unique<Device>();
	dev->path_ = path;
	dev->handler_ = this;
	dev->active_ = 1;
	dev->fd_ = fd;

	auto* ret = dev.get();
	devices_.push_back(std::move(dev));

	return ret;
}

bool DeviceHandler::releaseDeviceDBus(int devFD)
{
	struct stat st;
	if(fstat(devFD, &st) < 0 || !S_ISCHR(st.st_mode))
	{
		ny::sendWarning("DH::releaseDeviceDbus: failed to get stat struct for devFD");
	    return 0;
	}

	unsigned int majr = major(st.st_rdev);
	unsigned int minr = minor(st.st_rdev);

    //dbus release
    DBusMessage* msg;
    if (!(msg = dbus_message_new_method_call("org.freedesktop.login1", sessionPath().c_str(), 
		"org.freedesktop.login1.Session", "ReleaseDevice")))
    {
		ny::sendWarning("DH::releaseDeviceDBus: dbus_message_new_method_call failed");
        dbus_message_unref(msg);
        return 0;
    }

    if(!dbus_message_append_args(msg, DBUS_TYPE_UINT32, &majr, DBUS_TYPE_UINT32, &minr, 
		DBUS_TYPE_INVALID))
    {
		ny::sendWarning("DH::releaseDeviceDBus: dbus_message_append_args failed");
        dbus_message_unref(msg);
        return 0;
    }

    dbus_connection_send(&dbusHandler()->dbusConnection(), msg, nullptr);
    dbus_message_unref(msg);
	return 1;
}

bool DeviceHandler::releaseDeviceNormal(int fd)
{
	close(fd);
	return 1;
}

Device* DeviceHandler::deviceForFd(int fd) const
{
	for(auto& dev : devices_)
		if(dev->fd() == fd) return dev.get();

	return nullptr;
}

std::string DeviceHandler::sessionPath() const
{
	if(logind_)
		return logind_->sessionPath();

	return "";
}

void DeviceHandler::dbusPauseDevice(DBusMessage* msg)
{
    const char* type;
    unsigned int majr, minr;

    if(!dbus_message_get_args(msg, nullptr, DBUS_TYPE_UINT32, &majr, DBUS_TYPE_UINT32, 
		&minr, DBUS_TYPE_STRING, &type, DBUS_TYPE_INVALID))
    {
		ny::sendWarning("DH::dbusPauseDevice: dbus_message_get_args failed");
        return;
    }

    if(std::string(type) == "pause")
    {
        //pause it
        DBusMessage* m;
        if (!(m = dbus_message_new_method_call("org.freedesktop.login1", sessionPath().c_str(), 
			"org.freedesktop.login1.Session", "PauseDeviceComplete")))
        {
			ny::sendWarning("DH::dbusPauseDevice: dbus_message_new_method_call failed");
            return;
        }


        if (!dbus_message_append_args(m, DBUS_TYPE_UINT32, &majr, DBUS_TYPE_UINT32, &minr, 
			DBUS_TYPE_INVALID))
        {
			ny::sendWarning("DH::dbusPauseDevice: dbus_message_append_args failed");
            return;
        }

        dbus_connection_send(&dbusHandler()->dbusConnection(), m, nullptr);
        dbus_message_unref(m);
    }

    struct stat st;
    bool found = 0;
    for(auto& dev : devices_)
    {
        if(fstat(dev->fd_, &st) < 0 || !S_ISCHR(st.st_mode))
        {
			ny::sendWarning("DH::debusPauseDevice: failed to get stat struct for device fd");
            continue;
        }

        unsigned int fdMajor = major(st.st_rdev);
        unsigned int fdMinor = minor(st.st_rdev);

        if(fdMajor == majr && fdMinor == minr)
        {
            dev->active_ = 0;
            dev->pauseCallback_(*dev);
            found = 1;
        }
    }

    if(!found) ny::sendWarning("DH::dbusPauseDevice: device not found: ", majr, ".", minr);
}

void DeviceHandler::dbusResumeDevice(DBusMessage* msg)
{
    unsigned int majr, minr;
    if(!dbus_message_get_args(msg, nullptr, DBUS_TYPE_UINT32, &majr, DBUS_TYPE_UINT32, &minr, 
		DBUS_TYPE_INVALID))
    {
		ny::sendWarning("DH::dbusResumeDevice: dbus_message_get_args failed");
        return;
    }

    struct stat st;
    bool found = 0;
    for(auto& dev : devices_)
    {
        if(fstat(dev->fd_, &st) < 0 || !S_ISCHR(st.st_mode))
        {
			ny::sendWarning("DH::dbusResumeDevice: failed to get stat struct for device fd");
            continue;
        }

        unsigned int fdMajor = major(st.st_rdev);
        unsigned int fdMinor = minor(st.st_rdev);

        if(fdMajor == majr && fdMinor == minr)
        {
            dev->active_ = 1;
            dev->resumeCallback_(*dev);
            found = 1;
        }
    }

    if(!found) ny::sendWarning("DH::dbusResumeDevice: device not found");
}


}
