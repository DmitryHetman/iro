#include <iro/seat/seat.hpp>

#include <iro/seat/keyboard.hpp>
#include <iro/seat/pointer.hpp>
#include <iro/seat/touch.hpp>
#include <iro/seat/event.hpp>
#include <iro/compositor/compositor.hpp>
#include <iro/compositor/client.hpp>

#include <ny/base/log.hpp>
#include <nytl/make_unique.hpp>

#include <wayland-server-protocol.h>
#include <stdexcept>

namespace iro
{

//wayland interface
void seatGetPointer(wl_client*, wl_resource* resource, unsigned int id)
{
	SeatRes* seat = Resource::validateDisconnect<SeatRes>(resource, "seatGetPointer");
	if(!seat) return;

    seat->createPointer(id);
}
void seatGetKeyboard(wl_client*, wl_resource* resource, unsigned int id)
{
	SeatRes* seat = Resource::validateDisconnect<SeatRes>(resource, "seatGetKeyboard");
	if(!seat) return;

    seat->createKeyboard(id);
}
void seatGetTouch(wl_client*, wl_resource* resource, unsigned int id)
{
	SeatRes* seat = Resource::validateDisconnect<SeatRes>(resource, "seatGetTouch");
	if(!seat) return;

    seat->createTouch(id);
}

const struct wl_seat_interface seatImplementation
{
    &seatGetPointer,
    &seatGetKeyboard,
    &seatGetTouch
};

void bindSeat(wl_client* client, void* data, unsigned int version, unsigned int id)
{
	Seat* seat = static_cast<Seat*>(data);
	if(!seat)
	{
		ny::sendWarning("bindSeat: invalid data");
		return;
	}

    auto& clnt = seat->compositor().client(*client);
	clnt.addResource(nytl::make_unique<SeatRes>(*seat, *client, id, version));
}


//Seat implementation
Seat::Seat(Compositor& comp, const nytl::vec3b& caps) 
	: compositor_(&comp), name_("seat0"), pointer_(nullptr), keyboard_(nullptr), touch_(nullptr)
{
    wlGlobal_ = wl_global_create(&comp.wlDisplay(), &wl_seat_interface, wl_seat_interface.version, 
			this, &bindSeat);
	if(!wlGlobal_)
	{
		throw std::runtime_error("Seat::Seat: failed to create global");
		return;
	}

	if(caps[0]) pointer_ = nytl::make_unique<Pointer>(*this);
	if(caps[1]) keyboard_ = nytl::make_unique<Keyboard>(*this);
	//if(caps[2]) touch_ = nytl::make_unique<Touch>(*this);
}

Seat::~Seat()
{
}

///Seat resource
SeatRes::SeatRes(Seat& seat, wl_client& client, unsigned int id, unsigned int version) 
	: Resource(client, id, &wl_seat_interface, &seatImplementation, version), seat_(&seat)
{
	unsigned int capabilities = 0;
	if(seat.pointer()) capabilities |= WL_SEAT_CAPABILITY_POINTER;
	if(seat.keyboard()) capabilities |= WL_SEAT_CAPABILITY_KEYBOARD;
	if(seat.touch()) capabilities |= WL_SEAT_CAPABILITY_TOUCH;

    wl_seat_send_capabilities(&wlResource(), capabilities);
	wl_seat_send_name(&wlResource(), seat.name().c_str());
}

SeatRes::~SeatRes()
{
}

bool SeatRes::createPointer(unsigned int id)
{
	if(pointer_)
	{
		ny::sendWarning("seatRes::createPointer: tried to create second pointerRes");
		return 0;
	}

	if(!seat_->pointer())
	{
		ny::sendWarning("seatRes::createPointer: seat has no capability for pointer");
		return 0;
	}

	auto res = nytl::make_unique<PointerRes>(*this, id);
	if(!res)
	{
		ny::sendWarning("seatRes::createPointer: failed to create pointerRes");
		return 0;
	}

	res->onDestruction([=]{ this->pointer_ = nullptr; });
	pointer_ = res.get();

	client().addResource(std::move(res));
	return 1;
}

bool SeatRes::createKeyboard(unsigned int id)
{
	if(keyboard_)
	{
		ny::sendWarning("seatRes::createKeyboard: already exists");
		return 0;
	}

	if(!seat_->keyboard())
	{
		ny::sendWarning("seatRes::createKeyboard: seat has no capability for keyboard");
		return 0;
	}

	auto res = nytl::make_unique<KeyboardRes>(*this, id);
	if(!res)
	{
		ny::sendWarning("seatRes::createKeyboard: failed to create keyboardRes");
		return 0;
	}

	res->onDestruction([=]{ this->keyboard_ = nullptr; });
	keyboard_ = res.get();

	client().addResource(std::move(res));
	return 1;
}

bool SeatRes::createTouch(unsigned int id)
{
	/*
	if(touch_)
	{
		ny::sendWarning("seatRes::createTouch: already exists");
		return 0;
	}

	if(!seat_->touch())
	{
		ny::sendWarning("seatRes::createTouch: seat has no capability for touch");
		return 0;
	}

	auto res = nytl::make_unique<TouchRes>(*this, id);
	if(!res)
	{
		ny::sendWarning("seatRes::createTouch: failed to create touchRes");
		return 0;
	}

	res->onDestruction([=]{ this->touch_ = nullptr; });
	touch_ = res.get();

	client().addResource(std::move(res));
	return 1;
	*/

	return 0;
}


}
