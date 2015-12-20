#include <iro/seat/keyboard.hpp>

#include <iro/seat/seat.hpp>
#include <iro/seat/event.hpp>
#include <iro/compositor/surface.hpp>
#include <iro/compositor/client.hpp>
#include <iro/compositor/compositor.hpp>

#include <nytl/log.hpp>
#include <nytl/make_unique.hpp>

#include <wayland-server-protocol.h>

#include <linux/input.h>


namespace iro
{

//wayland implementation
void keyboardRelease(wl_client*, wl_resource* resource)
{
	KeyboardRes* kbres = Resource::validateDisconnect<KeyboardRes>(resource, "keyboardRelease");
	if(!kbres) return;

    kbres->destroy();
}

const struct wl_keyboard_interface keyboardImplementation
{
    &keyboardRelease
};

//Keyboard implementation
Keyboard::Keyboard(Seat& seat) : seat_(&seat)
{
}

Keyboard::~Keyboard()
{
}

Compositor& Keyboard::compositor() const
{
	return seat().compositor();
}

void Keyboard::sendKey(unsigned int key, bool press)
{
	nytl::sendLog("Key ", key, " ", press);

	//check grab
	if(grabbed_)
	{
		if(grab_.keyFunction)grab_.keyFunction(key, press);
		if(grab_.exclusive) return;
	}

	//exit on ESC key
	//only for debug - should NOT be in releases.
    if(key == KEY_ESC && press)
    {
		nytl::sendLog("ESC key - exiting compositor");
        compositor().exit();
        return;
    }

	//send key to client
    if(activeResource())
	{
		auto& ev = compositor().event(nytl::make_unique<KeyboardKeyEvent>(press, 
					key, &activeResource()->client()), 1);
		wl_keyboard_send_key(&activeResource()->wlResource(), ev.serial, 
			compositor().time(), key, press);
	}
     
	//callback	
    keyCallback_(key, press);
}

void Keyboard::sendFocus(SurfaceRes* newFocus)
{
    if(activeResource())
    {
        auto& ev = compositor().event(nytl::make_unique<KeyboardFocusEvent>(0, focus_.get(), 
				&focus_->client()), 1);
        wl_keyboard_send_leave(&activeResource()->wlResource(), ev.serial, &focus_->wlResource());
    }

	SurfaceRes* old = focus_.get();
    focus_.set(newFocus);

    if(activeResource())
    {
        auto& ev = compositor().event(nytl::make_unique<KeyboardFocusEvent>(1, focus_.get(), 
				&focus_->client()), 1);
        wl_keyboard_send_enter(&activeResource()->wlResource(), ev.serial, 
				&focus_->wlResource(), nullptr);
    }

    focusCallback_(old, newFocus);
}

KeyboardRes* Keyboard::activeResource() const
{
    return (focus_) ? focus_->client().keyboardResource() : nullptr;
}

bool Keyboard::grab(const Keyboard::Grab& grb, bool force)
{
	if(grabbed_)
	{
		if(!force) return 0;
		if(grab_.grabEndFunction)grab_.grabEndFunction(1);
	}

	grabbed_ = 1;
	grab_ = grb;
	return 1;
}

bool Keyboard::releaseGrab()
{
	if(grabbed_)
	{
		if(grab_.grabEndFunction)grab_.grabEndFunction(0);
		grabbed_ = 0;

		return 1;
	}

	return 0;
}

//Keyboard Resource
KeyboardRes::KeyboardRes(SeatRes& seatRes, unsigned int id)
	: Resource(seatRes.wlClient(), id, &wl_keyboard_interface, &keyboardImplementation, 
			seatRes.version())
{
}

Keyboard& KeyboardRes::keyboard() const
{
	return *seat().keyboard();
}

Seat& KeyboardRes::seat() const
{
	return seatRes().seat();
}

}
