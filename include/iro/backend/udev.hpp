#pragma once

#include <iro/include.hpp>
#include <nytl/nonCopyable.hpp>

//prototypes
struct udev;
struct udev_monitor;

namespace iro
{

///Wrapper around udev. Responsible for detecting new hotplugged devices and needed for creating
///an inputHandler (libinput).
class UDevHandler : public nytl::nonCopyable
{
protected:
	udev* udev_ = nullptr;
	udev_monitor* udevMonitor_ = nullptr;
	wl_event_source* udevEventSource_ = nullptr;

	static int eventCallback(int, unsigned int, void*);
	int udevEvent();

public:
	UDevHandler(Compositor& comp);
	~UDevHandler();

	udev& udevHandle() const { return *udev_; }
};

}
