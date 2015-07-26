#include <iro/backend/kms/input.hpp>

#include <iro/backend/session.hpp>
#include <iro/seat/seat.hpp>
#include <iro/seat/pointer.hpp>
#include <iro/seat/keyboard.hpp>
#include <iro/log.hpp>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int openRestricted(const char* path, int flags, void *data)
{
    sessionManager* hdl = (sessionManager*) data;

	device* dev = hdl->takeDevice(path);
	return (dev) ? dev->fd : -1;
}

void closeRestricted(int fd, void* data)
{
    sessionManager* hdl = (sessionManager*) data;

    hdl->releaseDevice(fd);
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
inputHandler::inputHandler(sessionManager& handler)
{
    udev_ = udev_new();
    udevMonitor_ = udev_monitor_new_from_netlink(udev_, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(udevMonitor_, "drm", nullptr);
    udev_monitor_filter_add_match_subsystem_devtype(udevMonitor_, "input", nullptr);
    udev_monitor_enable_receiving(udevMonitor_);
    wl_event_loop_add_fd(iroWlEventLoop(), udev_monitor_get_fd(udevMonitor_), WL_EVENT_READABLE, udevEventLoop, this);

    input_ = libinput_udev_create_context(&libinputImplementation, &handler, udev_);
    libinput_udev_assign_seat(input_, handler.getSeat().c_str());
    inputEventSource_ = wl_event_loop_add_fd(iroWlEventLoop(), libinput_get_fd(input_), WL_EVENT_READABLE, inputEventLoop, this);
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
                struct libinput_event_pointer* ev = libinput_event_get_pointer_event(event);

                double x = libinput_event_pointer_get_dx(ev);
                double y = libinput_event_pointer_get_dy(ev);
                vec2ui pos = iroPointer()->getPosition() + vec2ui(x,y);
                iroPointer()->sendMove(pos.x, pos.y);
                break;
            }
            case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
            {
                struct libinput_event_pointer* ev = libinput_event_get_pointer_event(event);

                iroPointer()->sendMove(libinput_event_pointer_get_absolute_x(ev), libinput_event_pointer_get_absolute_y(ev));
                break;
            }
            case LIBINPUT_EVENT_POINTER_BUTTON:
            {
                struct libinput_event_pointer* ev = libinput_event_get_pointer_event(event);
                unsigned int pressed = libinput_event_pointer_get_button_state(ev);

                if(pressed)iroPointer()->sendButtonPress(libinput_event_pointer_get_button(ev));
                else iroPointer()->sendButtonRelease(libinput_event_pointer_get_button(ev));

                break;
            }
            case LIBINPUT_EVENT_KEYBOARD_KEY:
            {
                struct libinput_event_keyboard* ev = libinput_event_get_keyboard_event(event);
                unsigned int pressed = libinput_event_keyboard_get_key_state(ev);

                if(pressed)iroKeyboard()->sendKeyPress(libinput_event_keyboard_get_key(ev));
                else iroKeyboard()->sendKeyRelease(libinput_event_keyboard_get_key(ev));

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
