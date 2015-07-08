#include <backend/kms/input.hpp>

#include <compositor/compositor.hpp>
#include <seat/seat.hpp>

int openRestricted(const char* path, int flags, void *data)
{
    return 0;
}

void closeRestricted(int fd, void* data)
{
}

const libinput_interface libinputImplementation =
{
    openRestricted,
    closeRestricted
};

///////////////////////////////////////////////
int inputEventLoop(int fd, unsigned int mask, void* data)
{
    inputHandler* handler = (inputHandler*) data;
    return handler->inputEvent();
}

int udevEventLoop(int fd, unsigned int mask, void* data)
{
    inputHandler* handler = (inputHandler*) data;
    return handler->udevEvent();
}

///////////////////////////////////////////////
inputHandler::inputHandler()
{
    udev_ = udev_new();

    udevMonitor_ = udev_monitor_new_from_netlink(udev_, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(udevMonitor_, "drm", nullptr);
    udev_monitor_filter_add_match_subsystem_devtype(udevMonitor_, "input", nullptr);
    udev_monitor_enable_receiving(udevMonitor_);

    wl_event_loop_add_fd(getCompositor()->getWlEventLoop(), udev_monitor_get_fd(udevMonitor_), WL_EVENT_READABLE, udevEventLoop, this);

    input_ = libinput_udev_create_context(&libinputImplementation, this, udev_);
    libinput_udev_assign_seat(input_, "seat0");
    inputEventSource_ = wl_event_loop_add_fd(getCompositor()->getWlEventLoop(), libinput_get_fd(input_), WL_EVENT_READABLE, inputEventLoop, this);
}

inputHandler::~inputHandler()
{
    libinput_unref(input_);
    udev_unref(udev_);
}

int inputHandler::inputEvent()
{
    return 0;
}

int inputHandler::udevEvent()
{
    return 0;
}
