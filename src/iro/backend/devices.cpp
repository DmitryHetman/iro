#include <iro/backend/devices.hpp>
//#include <iro/backend/logind.hpp>
//#include <iro/backend/dbus.hpp>
#include <iro/compositor/compositor.hpp>

#include <nytl/log.hpp>
#include <nytl/make_unique.hpp>

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
	handler_->destroyDevice(*this);
}

//DeviceHandler
DeviceHandler::DeviceHandler(bool useRoot)
{
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

DeviceHandler::DeviceHandler(Compositor& comp, LogindHandler& logind)
{
	nytl::sendWarning("DeviceHandler with logind not implented yet");
}

DeviceHandler::~DeviceHandler()
{
	nytl::sendLog("Finished ~DeviceHandler, ", devices_.size(), " left.");
	*nytl::sendLog.stream << std::endl;
}

Device* DeviceHandler::createDevice(const std::string& path, int flags)
{
	int fd = open(path.c_str(), flags | O_CLOEXEC);
	if(fd < 0)
	{
		nytl::sendWarning("DeviceHandler::createDevice: failed with code ", fd);
		return nullptr;
	}

	auto dev = nytl::make_unique<Device>();
	dev->path_ = path;
	dev->handler_ = this;
	dev->fd_ = fd;
	auto* ret = dev.get();

	devices_.push_back(std::move(dev));
	return ret;
}

bool DeviceHandler::destroyDevice(const Device& dev)
{
	for(auto it = devices_.cbegin(); it != devices_.cend(); ++it)
	{
		if((*it).get() == &dev)
		{
			close(dev.fd());
			devices_.erase(it);
			return 1;
		}
	}

	nytl::sendWarning("DeviceHandler::destroyDevice: invalid device parameter");
	return 0;
}

}
