#include <backend/kms/input.hpp>

#include <compositor/compositor.hpp>
#include <seat/seat.hpp>
#include <seat/pointer.hpp>
#include <log.hpp>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <iostream>

int openRestricted(const char* path, int flags, void *data)
{
	int fd;
	fd = open(path, flags);
	return fd < 0 ? -errno : fd;
}

void closeRestricted(int fd, void* data)
{
    close(fd);
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
    libinput_dispatch(input_);

    struct libinput_event* event;
    while((event = libinput_get_event(input_)))
    {
        switch(libinput_event_get_type(event))
        {
            case LIBINPUT_EVENT_POINTER_MOTION:
            {
                iroLog << "motion1" << std::endl;
                struct libinput_event_pointer* ev = libinput_event_get_pointer_event(event);
                double x = libinput_event_pointer_get_dx(ev);
                double y = libinput_event_pointer_get_dy(ev);
                vec2ui pos = getSeat()->getPointer()->getPosition() + vec2ui(x,y);
                getSeat()->getPointer()->sendMove(pos.x, pos.y);
                break;
            }
            case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
            {
                iroLog << "motion2" << std::endl;
                struct libinput_event_pointer* ev = libinput_event_get_pointer_event(event);
                getSeat()->getPointer()->sendMove(libinput_event_pointer_get_absolute_x(ev), libinput_event_pointer_get_absolute_y(ev));

                break;
            }
            case LIBINPUT_EVENT_KEYBOARD_KEY:
            {
                break;
            }
            default:
                break;
        }

        libinput_event_destroy(event);
    }

    return 0;
}

int inputHandler::udevEvent()
{
    std::cout << "udev event" << std::endl;
    return 0;
}
