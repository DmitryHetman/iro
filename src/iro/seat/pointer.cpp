#include <iro/seat/pointer.hpp>

#include <iro/seat/seat.hpp>
#include <iro/seat/event.hpp>
#include <iro/seat/keyboard.hpp>
#include <iro/seat/cursorSurface.hpp>
#include <iro/compositor/compositor.hpp>
#include <iro/compositor/surface.hpp>
#include <iro/compositor/client.hpp>
#include <iro/backend/backend.hpp>
#include <iro/backend/output.hpp>

#include <ny/base/log.hpp>

#include <wayland-server-protocol.h>

namespace iro
{

//wayland implementation
void pointerSetCursor(wl_client*, wl_resource* resource, unsigned int serial, 
		wl_resource* surface, int hx, int hy)
{
	//todo: serial and stuff
	PointerRes* ptr = Resource::validateDisconnect<PointerRes>(resource, "setCursor");
	if(!ptr) return;

	if(!ptr->pointer().activeResource() || 
			&ptr->client() != &ptr->pointer().activeResource()->client())
	{
		ny::sendWarning("pointerSetCursor: requesting client does not have pointer focus");
		return;
	}

	if(!surface)
	{
		ptr->pointer().resetCursor();
		return;
	}

	SurfaceRes* surf = Resource::validateDisconnect<SurfaceRes>(surface, "setCursor2");
	if(!surf) return;

	if(surf->roleType() != surfaceRoleType::none && surf->roleType() != surfaceRoleType::cursor)
	{
		ny::sendWarning("pointerSetCursor: invalid surface role");
		wl_resource_post_error(resource, WL_POINTER_ERROR_ROLE, "invalid role");
		return;
	}

	ptr->pointer().cursor(*surf, {hx, hy});
}

void pointerRelease(wl_client*, wl_resource* resource)
{
	PointerRes* ptr = Resource::validateDisconnect<PointerRes>(resource, "pointerRelease");
	if(!ptr) return;

	ptr->destroy();
}

const struct wl_pointer_interface pointerImplementation
{
    &pointerSetCursor,
    &pointerRelease
};

//Pointer implementation
Pointer::Pointer(Seat& s) : seat_(&s)
{
}

Pointer::~Pointer()
{
}

Compositor& Pointer::compositor() const
{
	return seat().compositor();
}

void Pointer::setOver(SurfaceRes* newOne)
{
    //send leave
    if(over_.get())
    {
		auto* ptrRes = over_->client().pointerResource();

        if(!ptrRes)
		{
			ny::sendWarning("pointer::sendActive: left surface without pointerRes");
		}
        else
        {
            auto& ev = compositor().event(std::make_unique<PointerFocusEvent>(0, over_.get()), 1);
            wl_pointer_send_leave(&ptrRes->wlResource(), ev.serial, &over_->wlResource());
        }
    }

    SurfaceRes* old = over_.get();
    over_.reset(newOne);

    //send enter
    if(newOne)
    {
		auto* ptrRes = newOne->client().pointerResource();

        if(!ptrRes)
		{
			ny::sendWarning("pointer::sendActive: Entered surface without pointerRes");
		}
        else
        {
			nytl::Vec2i pos = wlFixedPositionRelative(); 

			auto& ev = compositor().event(std::make_unique<PointerFocusEvent>(1, newOne), 1);
            wl_pointer_send_enter(&ptrRes->wlResource(), ev.serial, &newOne->wlResource(), 
					pos.x, pos.y);
        }
    }

	//if(cursor_.get() && &cursor_->client() != &newOne->client()) cursor_.reset();
    focusCallback_(old, newOne);
}

void Pointer::sendMove(const nytl::Vec2i& pos)
{
	//todo
	//redraw, position and stuff
	auto cpos = nytl::clamp(pos, nytl::Vec2i{0, 0}, nytl::Vec2i{1920, 1080});
	nytl::Vec2i delta = cpos - position_;
	position_ = cpos;

	if(!compositor().backend())
	{
		ny::sendWarning("pointer::sendMove: invalid backend, cant get output");
		return;
	}

	Output* overOut = compositor().backend()->outputAt(position_);
	if(!overOut) return;

	overOut->scheduleRepaint();

	//grab check
	if(grabbed_)
	{
		if(grab_.moveFunction)grab_.moveFunction(position_, delta);
		if(grab_.exclusive)
		{
			return;
		}
	}

	//focus check	
	SurfaceRes* surf = overOut->surfaceAt(position_);
	if(surf != over_.get()) setOver(surf);

	//send movement to client
	if(activeResource())
	{
		auto wlpos = wlFixedPositionRelative();
		wl_pointer_send_motion(&activeResource()->wlResource(), compositor().time(), 
				wlpos.x, wlpos.y);
	}	
	
	//callbacks
	moveCallback_(position_, delta);
}

void Pointer::sendButton(unsigned int button, bool press)
{
	//grab check
	if(grabbed_)
	{
		if(grab_.buttonFunction)grab_.buttonFunction(button, press);
		if(grab_.exclusive) return;
	}

	//send button to client
	if(activeResource())
	{
		auto& ev = compositor().event(std::make_unique<PointerButtonEvent>(press,
				button, &activeResource()->client()), 1);

		wl_pointer_send_button(&activeResource()->wlResource(), ev.serial, compositor().time(), 
				button, press);

	}

	if(press && seat().keyboard())
	{
		seat().keyboard()->sendFocus(over_.get());
	}

	//callbacks
	buttonCallback_(button, press);
}

void Pointer::sendAxis(unsigned int axis, double value)
{
	//grab check
	if(grabbed_)
	{
		if(grab_.axisFunction)grab_.axisFunction(axis, value);
		if(grab_.exclusive) return;
	}

	//send axis to client
    if(activeResource())
	{
		wl_pointer_send_axis(&activeResource()->wlResource(), compositor().time(), axis, value);
	}

	//callback
    axisCallback_(axis, value);
}

void Pointer::cursor(SurfaceRes& surf, const nytl::Vec2i& hotspot)
{
    if(surf.roleType() != surfaceRoleType::cursor)
    {
		if(surf.roleType() == surfaceRoleType::none)
		{
			surf.role(std::make_unique<CursorSurfaceRole>(*this, surf));
		}
		else
		{
			ny::sendWarning("pointer::setCursor: cursor has different role");
			return;
		}
    }

	auto* crole = static_cast<CursorSurfaceRole*>(surf.role());
	crole->hotspot(hotspot);

    cursor_.reset(&surf);
}

void Pointer::resetCursor()
{
	cursor_.reset(nullptr);
}

nytl::Vec2i Pointer::wlFixedPosition() const
{
    return nytl::Vec2i(wl_fixed_from_int(position_.x), wl_fixed_from_int(position_.y));
}

nytl::Vec2i Pointer::wlFixedPositionRelative() const
{
	if(!over_) return {};

	auto pos = over_->position();
	return wlFixedPosition() - nytl::Vec2i(wl_fixed_from_int(pos.x), wl_fixed_from_int(pos.y));
}

PointerRes* Pointer::activeResource() const
{
    return (over_) ? over_->client().pointerResource() : nullptr;
}

bool Pointer::grab(const Pointer::Grab& grb, bool force)
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

bool Pointer::releaseGrab()
{
	if(grabbed_)
	{
		if(grab_.grabEndFunction)grab_.grabEndFunction(0);
		grabbed_ = 0;

		return 1;
	}

	return 0;
}

//pointer resource
PointerRes::PointerRes(SeatRes& seatRes, unsigned int id)
	: Resource(seatRes.wlClient(), id, &wl_pointer_interface, &pointerImplementation, 
		seatRes.version()), seatRes_(&seatRes)
{
}

Seat& PointerRes::seat() const
{
	return seatRes().seat();
}

Pointer& PointerRes::pointer() const
{
	return *seat().pointer();
}

}
