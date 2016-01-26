#include <iro/backend/udev.hpp>
#include <iro/compositor/compositor.hpp>

#include <ny/base/log.hpp>

#include <wayland-server-core.h>
#include <libudev.h>

namespace iro
{

UDevHandler::UDevHandler(Compositor& comp)
{
    udev_ = udev_new();
    udevMonitor_ = udev_monitor_new_from_netlink(udev_, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(udevMonitor_, "drm", nullptr);
    udev_monitor_filter_add_match_subsystem_devtype(udevMonitor_, "input", nullptr);
    udev_monitor_enable_receiving(udevMonitor_);
    wl_event_loop_add_fd(&comp.wlEventLoop(), udev_monitor_get_fd(udevMonitor_), 
			WL_EVENT_READABLE, eventCallback, this);
}

UDevHandler::~UDevHandler()
{
	if(udev_) udev_unref(udev_);
	if(udevEventSource_) wl_event_source_remove(udevEventSource_);
}

int UDevHandler::eventCallback(int, unsigned int, void* data)
{
	if(!data) return 1;
	return static_cast<UDevHandler*>(data)->udevEvent();
}

int UDevHandler::udevEvent()
{
	ny::sendLog("received udev event");
	return 1;
}

}
