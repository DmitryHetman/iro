#include <iro/backend/input.hpp>
#include <iro/backend/udev.hpp>
#include <iro/backend/devices.hpp>
#include <iro/compositor/compositor.hpp>
#include <iro/seat/seat.hpp>
#include <iro/seat/pointer.hpp>
#include <iro/seat/keyboard.hpp>

#include <nytl/log.hpp>
#include <nytl/vec.hpp>

#include <wayland-server-core.h>

#include <libinput.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <errno.h>

namespace iro
{

int openRestricted(const char* pth, int flags, void *data)
{
	if(!data) return -1;

    InputHandler* hdl = static_cast<InputHandler*>(data);
	Device* dev = hdl->deviceHandler().takeDevice(pth, flags);

	return (dev && dev->active() && dev->fd() > 0) ? dev->fd() : -1;
}

void closeRestricted(int fd, void* data)
{
	if(!data) return;

    InputHandler* hdl = static_cast<InputHandler*>(data);
	auto* device = hdl->deviceHandler().deviceForFd(fd);

	if(!device)
	{
		nytl::sendWarning("InputHandler::closeRestricted: could not find device for fd");
		return;
	}

    hdl->deviceHandler().releaseDevice(*device);
}

void libinputLogHandler(struct libinput *input, enum libinput_log_priority priority, 
		const char *format, va_list args)
{
	nytl::sendLog("libinput error: "); //todo man
}

const libinput_interface libinputImpl =
{
    openRestricted,
    closeRestricted
};

//InputHandler
InputHandler::InputHandler(Compositor& comp, Seat& seat, UDevHandler& udev, DeviceHandler& dev)
	: seat_(&seat), deviceHandler_(&dev)
{
    libinput_ = libinput_udev_create_context(&libinputImpl, this, &udev.udevHandle());

	const char *xdg_seat = getenv("XDG_SEAT");
    libinput_udev_assign_seat(libinput_, xdg_seat);

	libinput_log_set_handler(libinput_, &libinputLogHandler);
	libinput_log_set_priority(libinput_, LIBINPUT_LOG_PRIORITY_ERROR);

    inputEventSource_ = wl_event_loop_add_fd(&comp.wlEventLoop(), libinput_get_fd(libinput_), 
			WL_EVENT_READABLE, inputEventCallback, this);

	nytl::sendLog("inputHandler set up");
}

InputHandler::~InputHandler()
{
    if(libinput_)libinput_unref(libinput_);
	if(inputEventSource_)wl_event_source_remove(inputEventSource_);
}

int InputHandler::inputEventCallback(int, unsigned int, void* data)
{
	if(!data) return 1;
	return static_cast<InputHandler*>(data)->inputEvent();
}

int InputHandler::inputEvent()
{
	Pointer* pointer = seat().pointer();
	Keyboard* keyboard = seat().keyboard();

	nytl::sendLog("InputHandler event");
    libinput_dispatch(libinput_);

    struct libinput_event* event;
    while((event = libinput_get_event(libinput_)))
    {
        switch(libinput_event_get_type(event))
        {
            case LIBINPUT_EVENT_POINTER_MOTION:
            {
				if(!pointer) break;
                struct libinput_event_pointer* ev = libinput_event_get_pointer_event(event);

                double x = libinput_event_pointer_get_dx(ev);
                double y = libinput_event_pointer_get_dy(ev);
				nytl::vec2f pos = pointer->position() + nytl::vec2f(x,y);
                pointer->sendMove(pos);
                break;
            }
            case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
            {
				if(!pointer) break;
                struct libinput_event_pointer* ev = libinput_event_get_pointer_event(event);

				nytl::vec2f position;
				position.x = libinput_event_pointer_get_absolute_x(ev);
				position.y = libinput_event_pointer_get_absolute_y(ev);
                pointer->sendMove(position);
                break;
            }
            case LIBINPUT_EVENT_POINTER_BUTTON:
            {
				if(!pointer) break;
                struct libinput_event_pointer* ev = libinput_event_get_pointer_event(event);
                unsigned int pressed = libinput_event_pointer_get_button_state(ev);

				pointer->sendButton(libinput_event_pointer_get_button(ev), pressed);
                break;
            }
            case LIBINPUT_EVENT_KEYBOARD_KEY:
            {
				if(!keyboard) break;
                struct libinput_event_keyboard* ev = libinput_event_get_keyboard_event(event);
                unsigned int pressed = libinput_event_keyboard_get_key_state(ev);

                keyboard->sendKey(libinput_event_keyboard_get_key(ev), pressed);
                break;
            }
            default:
				nytl::sendLog("InputHandler: unhandeled event");
                break;
        }

        libinput_event_destroy(event);
    }

    return 0;
}

}
