#include <iro/seat/pointer.hpp>

#include <iro/seat/seat.hpp>
#include <iro/seat/event.hpp>
#include <iro/seat/cursorSurface.hpp>
#include <iro/compositor/compositor.hpp>
#include <iro/compositor/surface.hpp>
#include <iro/compositor/client.hpp>
#include <iro/backend/backend.hpp>
#include <iro/backend/output.hpp>

#include <nytl/log.hpp>
#include <nytl/make_unique.hpp>

#include <wayland-server-protocol.h>

namespace iro
{

//wayland implementation
void pointerSetCursor(wl_client*, wl_resource* resource, unsigned int serial, 
		wl_resource* surface, int hx, int hy)
{
	PointerRes* ptr = Resource::validateDisconnect<PointerRes>(resource, "setCursor");
	if(!ptr) return;

	if(!ptr->pointer().activeResource() || 
			&ptr->client() != &ptr->pointer().activeResource()->client())
	{
		nytl::sendWarning("pointerSetCursor: requesting client does not have pointer focus");
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
		nytl::sendWarning("pointerSetCursor: invalid surface role");
		wl_resource_post_error(resource, WL_POINTER_ERROR_ROLE, "invalid role");
		return;
	}

	ptr->pointer().cursor(*surf, nytl::vec2i(hx, hy));
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
		auto* surf = over_.get();
		auto* ptrRes = surf->client().pointerResource();

        if(!ptrRes)
		{
			nytl::sendWarning("pointer::sendActive: left surface without pointerRes");
		}
        else
        {
            auto& ev = compositor().event(nytl::make_unique<PointerFocusEvent>(0, surf), 1);
            wl_pointer_send_leave(&ptrRes->wlResource(), ev.serial, &surf->wlResource());
        }
    }

    SurfaceRes* old = over_.get();
    over_.set(newOne);

    //send enter
    if(newOne)
    {
		auto* ptrRes = newOne->client().pointerResource();

        if(!ptrRes)
		{
			nytl::sendWarning("pointer::sendActive: Entered surface without pointerRes");
		}
        else
        {
			nytl::vec2i pos; //todo, relative pos

			auto& ev = compositor().event(nytl::make_unique<PointerFocusEvent>(1, newOne), 1);
            wl_pointer_send_enter(&ptrRes->wlResource(), ev.serial, &newOne->wlResource(), 
					pos.x, pos.y);
        }
    }

    focusCallback_(old, newOne);
}

void Pointer::sendMove(const nytl::vec2i& pos)
{
	//nytl::vec2i delta = pos - position_;
	position_ = pos;

	moveCallback_(position_);

	if(!compositor().backend())
	{
		nytl::sendWarning("pointer::sendMove: invalid backend, cant get output");
		return;
	}

	Output* overOut = compositor().backend()->outputAt(position_);
	if(!overOut) return;

	if(!seat().modeEvent())
	{
		SurfaceRes* surf = overOut->surfaceAt(position_);
		if(surf != over_.get()) setOver(surf);

		if(activeResource())
		{
			auto wlpos = wlFixedPosition();
			wl_pointer_send_motion(&activeResource()->wlResource(), compositor().time(), 
					wlpos.x, wlpos.y);
		}	
	}
}

void Pointer::sendButton(unsigned int button, bool press)
{
	if(activeResource() && !seat().modeEvent())
	{
		auto& ev = compositor().event(nytl::make_unique<PointerButtonEvent>(press,
				button, &activeResource()->client()), 1);

		wl_pointer_send_button(&activeResource()->wlResource(), ev.serial, compositor().time(), 
				button ,press);
	}

	buttonCallback_(button, press);
}

void Pointer::sendAxis(unsigned int axis, double value)
{
    if(activeResource() && !seat().modeEvent())
	{
		wl_pointer_send_axis(&activeResource()->wlResource(), compositor().time(), axis, value);
	}

    axisCallback_(axis, value);
}

void Pointer::cursor(SurfaceRes& surf, const nytl::vec2i& hotspot)
{
    if(surf.roleType() != surfaceRoleType::cursor)
    {
		if(surf.roleType() == surfaceRoleType::none)
		{
			surf.role(nytl::make_unique<CursorSurfaceRole>());
		}
		else
		{
			nytl::sendWarning("pointer::setCursor: cursor has different role");
			return;
		}
    }

	auto* crole = static_cast<CursorSurfaceRole*>(surf.role());
	crole->hotspot(hotspot);

    cursor_.set(&surf);
}

void Pointer::resetCursor()
{
	cursor_.set(nullptr);
}

nytl::vec2i Pointer::wlFixedPosition() const
{
    return nytl::vec2i(wl_fixed_from_int(position_.x), wl_fixed_from_int(position_.y));
}

PointerRes* Pointer::activeResource() const
{
    return (over_.get()) ? over_.get()->client().pointerResource() : nullptr;
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
