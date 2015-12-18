#pragma once

#include <iro/include.hpp>

struct libinput;

namespace iro
{

///Wrapper around libinput that initializes and manages the libinput resources.
class InputHandler
{
protected:
	Seat* seat_;
	DeviceHandler* deviceHandler_;

    libinput* libinput_ = nullptr;
    wl_event_source* inputEventSource_ = nullptr;

    static int inputEventCallback(int, unsigned int, void*);
	int inputEvent();

public:
    InputHandler(Compositor& comp, Seat& seat, UDevHandler& udev, DeviceHandler& dev);
    ~InputHandler();

	DeviceHandler& deviceHandler() const { return *deviceHandler_; }
	Seat& seat() const { return *seat_; }
};

}
