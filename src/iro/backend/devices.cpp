#include <iro/backend/devices.hpp>
#include <iro/backend/dbus.hpp>
#include <iro/backend/logind.hpp>
#include <iro/compositor/compositor.hpp>

#include <nytl/log.hpp>
#include <nytl/make_unique.hpp>

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
			nytl::sendWarning("DeviceHandler::DeviceHandler: geteuid != 0");
			return;
		}
	
		int sockpair[2];
		if (socketpair(AF_LOCAL, SOCK_SEQPACKET | SOCK_CLOEXEC, 0, sockpair) != 0)
		{
			nytl::sendWarning("DeviceHandler::DeviceHandler: socketpair failed");
			return;
		}

		pid_t child = fork();
		if(child < 0)
		{
			nytl::sendWarning("DeviceHandler::DeviceHandler: fork failed");
			return;
		}
		else if(child == 0)
		{
			close(sockpair[0]);
			forkSocket_ = sockpair[1];

			//nytl::sendLog("DeviceHandler: fork exiting");
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
    dbus.addSignalMatch(path, "org.freedesktop.DBus.Properties", "PropertiesChanged", 
		logind.sessionPath(), nytl::memberCallback(&DeviceHandler::dbusPropertiesChanged, this));
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
		nytl::sendWarning("DeviceHandler::releaseDevice: device not found.");
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
		nytl::sendWarning("DeviceHandler::takeDeviceDBus: failed to get stat struct for path");
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
		nytl::sendWarning("DeviceHandler::takeDeviceDBus: dbus_message_new_method_call failed");
        dbus_message_unref(msg);
        return nullptr;
    }

    if(!dbus_message_append_args(msg, DBUS_TYPE_UINT32, &majr, DBUS_TYPE_UINT32, &minr, 
		DBUS_TYPE_INVALID))
    {
		nytl::sendWarning("DH::takeDeviceDBus: dbus_message_append_args failed");
        dbus_message_unref(msg);
        return nullptr;
    }

    DBusMessage *reply;
    if(!(reply = dbus_connection_send_with_reply_and_block(&dbusHandler()->dbusConnection(), msg, 
		-1, &error)))
    {
		DBusHandler::checkError(error, errStr);
		nytl::sendWarning("DH::takeDevDBus: dbus_connection_send_with_reply_and_block: ", errStr);
        dbus_message_unref(msg);
        return nullptr;
    }

    dbus_bool_t paused;
    if(!dbus_message_get_args(reply, nullptr, DBUS_TYPE_UNIX_FD, &fd, DBUS_TYPE_BOOLEAN, 
		&paused, DBUS_TYPE_INVALID))
    {
		nytl::sendWarning("DH::takeDeviceDBus: dbus_message_get_args failed");
        dbus_message_unref(reply);
        dbus_message_unref(msg);
        return nullptr;
    }

    int fl;
    if((fl = fcntl(fd, F_GETFL)) < 0 || fcntl(fd, F_SETFD, fl | FD_CLOEXEC) < 0)
    {
		nytl::sendWarning("DH::takeDeviceDBus: invalid fd");
        close(fd);
        dbus_message_unref(reply);
        dbus_message_unref(msg);
        return nullptr;
    }

    dbus_message_unref(reply);
    dbus_message_unref(msg);

	auto dev = nytl::make_unique<Device>();
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
		nytl::sendWarning("DeviceHandler::takeDeviceNormal: open failed with code ", fd);
		return nullptr;
	}

	auto dev = nytl::make_unique<Device>();
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
		nytl::sendWarning("DH::releaseDeviceDbus: failed to get stat struct for devFD");
	    return 0;
	}

	unsigned int majr = major(st.st_rdev);
	unsigned int minr = minor(st.st_rdev);

    //dbus release
    DBusMessage* msg;
    if (!(msg = dbus_message_new_method_call("org.freedesktop.login1", sessionPath().c_str(), 
		"org.freedesktop.login1.Session", "ReleaseDevice")))
    {
		nytl::sendWarning("DH::releaseDeviceDBus: dbus_message_new_method_call failed");
        dbus_message_unref(msg);
        return 0;
    }

    if(!dbus_message_append_args(msg, DBUS_TYPE_UINT32, &majr, DBUS_TYPE_UINT32, &minr, 
		DBUS_TYPE_INVALID))
    {
		nytl::sendWarning("DH::releaseDeviceDBus: dbus_message_append_args failed");
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
		nytl::sendWarning("DH::dbusPauseDevice: dbus_message_get_args failed");
        return;
    }

    if(std::string(type) == "pause")
    {
        //pause it
        DBusMessage* m;
        if (!(m = dbus_message_new_method_call("org.freedesktop.login1", sessionPath().c_str(), 
			"org.freedesktop.login1.Session", "PauseDeviceComplete")))
        {
			nytl::sendWarning("DH::dbusPauseDevice: dbus_message_new_method_call failed");
            return;
        }


        if (!dbus_message_append_args(m, DBUS_TYPE_UINT32, &majr, DBUS_TYPE_UINT32, &minr, 
			DBUS_TYPE_INVALID))
        {
			nytl::sendWarning("DH::dbusPauseDevice: dbus_message_append_args failed");
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
			nytl::sendWarning("DH::debusPauseDevice: failed to get stat struct for device fd");
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

    if(!found) nytl::sendWarning("DH::dbusPauseDevice: device not found: ", majr, ".", minr);
}

void DeviceHandler::dbusResumeDevice(DBusMessage* msg)
{
    unsigned int majr, minr;
    if(!dbus_message_get_args(msg, nullptr, DBUS_TYPE_UINT32, &majr, DBUS_TYPE_UINT32, &minr, 
		DBUS_TYPE_INVALID))
    {
		nytl::sendWarning("DH::dbusResumeDevice: dbus_message_get_args failed");
        return;
    }

    struct stat st;
    bool found = 0;
    for(auto& dev : devices_)
    {
        if(fstat(dev->fd_, &st) < 0 || !S_ISCHR(st.st_mode))
        {
			nytl::sendWarning("DH::dbusResumeDevice: failed to get stat struct for device fd");
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

    if(!found) nytl::sendWarning("DH::dbusResumeDevice: device not found");
}

void DeviceHandler::dbusPropertiesChanged(DBusMessage* m)
{
	nytl::sendLog("props changes");

    DBusMessageIter iter;
    if (!dbus_message_iter_init(m, &iter) || dbus_message_iter_get_arg_type(&iter) != 
			DBUS_TYPE_STRING) 
		goto error0;

    const char *interface;
    dbus_message_iter_get_basic(&iter, &interface);

    if (!dbus_message_iter_next(&iter) || 
			dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
        goto error0;

    DBusMessageIter sub;
    dbus_message_iter_recurse(&iter, &sub);

    DBusMessageIter entry;
    while (dbus_message_iter_get_arg_type(&sub) == DBUS_TYPE_DICT_ENTRY)
    {
        dbus_message_iter_recurse(&sub, &entry);

        if(dbus_message_iter_get_arg_type(&entry) != DBUS_TYPE_STRING)
            goto error0;

        const char *name;
        dbus_message_iter_get_basic(&entry, &name);
        if(!dbus_message_iter_next(&entry))
            goto error0;

        if(std::string(name) == "Active")
        {
            dbusParseActive(m, &entry);
            return;
        }

        dbus_message_iter_next(&sub);
    }

    if (!dbus_message_iter_next(&iter) || 
			dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
        goto error0;

    dbus_message_iter_recurse(&iter, &sub);

    while (dbus_message_iter_get_arg_type(&sub) == DBUS_TYPE_STRING)
    {
        const char *name;
        dbus_message_iter_get_basic(&sub, &name);

        if(std::string(name) == "Active")
        {
            dbusGetActive();
            return;
        }

        dbus_message_iter_next(&sub);
    }

    return;

error0:
	nytl::sendWarning("DeviceHandler: cannot parse PropertiesChanged dbus signal");
}

void DeviceHandler::dbusGetActiveCallback(DBusPendingCall* pending, void* data)
{
	if(!data)
	{
		nytl::sendWarning("nytl error dummy");
		return;
	}
	DeviceHandler* dh = static_cast<DeviceHandler*>(data);

	dbus_pending_call_unref(dh->pendingActive_);
	dh->pendingActive_ = nullptr;

	DBusMessage *m;
	if (!(m = dbus_pending_call_steal_reply(pending)))
	{
		nytl::sendWarning("nytl error dummy");
		return;
	}

	DBusMessageIter iter;
	if(dbus_message_get_type(m) == DBUS_MESSAGE_TYPE_METHOD_RETURN && 
		dbus_message_iter_init(m, &iter))
      dh->dbusParseActive(m, &iter);

   dbus_message_unref(m);
}

void DeviceHandler::dbusGetActive()
{
	DBusMessage *m;
   	if(!(m = dbus_message_new_method_call("org.freedesktop.login1", sessionPath().c_str(), 
			"org.freedesktop.DBus.Properties", "Get")))
	{
		nytl::sendWarning("nytl error dummy");
		return;
	}

   	const char *iface = "org.freedesktop.login1.Session";
  	const char *name = "Active";
   	if (!dbus_message_append_args(m, DBUS_TYPE_STRING, &iface, DBUS_TYPE_STRING, 
		&name, DBUS_TYPE_INVALID))
	{
		nytl::sendWarning("nytl error dummy");
		dbus_message_unref(m);
		return;
	}

	DBusPendingCall *pending;
	if (!dbus_connection_send_with_reply(&dbusHandler()->dbusConnection(), m, &pending, -1))
	{
		nytl::sendWarning("nytl error dummy");
		dbus_message_unref(m);
		return;
	}

	if (!dbus_pending_call_set_notify(pending, &DeviceHandler::dbusGetActiveCallback, 
				this, nullptr))
	{
		nytl::sendWarning("nytl error dummy");
		dbus_pending_call_cancel(pending);
		dbus_pending_call_unref(pending);
		dbus_message_unref(m);
		return;
	}

	if (pendingActive_)
	{
		dbus_pending_call_cancel(pendingActive_);
		dbus_pending_call_unref(pendingActive_);
	}

	pendingActive_ = pending;
}

void DeviceHandler::dbusParseActive(DBusMessage* msg, DBusMessageIter* it)
{
	nytl::sendLog("DBUS::parsteactive");
}

}
