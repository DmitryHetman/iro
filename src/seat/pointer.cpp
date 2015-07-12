#include <seat/pointer.hpp>

#include <seat/seat.hpp>
#include <resources/surface.hpp>
#include <backend/backend.hpp>
#include <backend/output.hpp>

#include <wayland-server-protocol.h>

//////////////////////////
void pointerSetCursor(wl_client* client, wl_resource* resource, unsigned int serial, wl_resource* surface, int hotspot_x, int hotspot_y)
{
    pointerRes* res = (pointerRes*) wl_resource_get_user_data(resource);
    pointer* p = res->getPointer();

    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(surface);

    if(p->getGrab() == res) p->setCursor(surf, vec2ui(hotspot_x, hotspot_y));
}
void pointerRelease(wl_client* client, wl_resource* resource)
{
    pointerRes* res = (pointerRes*) wl_resource_get_user_data(resource);
    res->destroy();
}

const struct wl_pointer_interface pointerImplementation
{
    &pointerSetCursor,
    &pointerRelease
};

////////////////////////////////////777
pointer::pointer(seat* s) : seat_(s), grab_(nullptr)
{
}

pointer::~pointer()
{

}

void pointer::sendMove(unsigned int x, unsigned int y)
{
    position_.x = x;
    position_.y = y;

    getBackend()->getOutput()->refresh();

    if(!grab_ || !over_)
        return;

    wl_fixed_t fx = wl_fixed_from_int(x - over_->getPosition().x);
    wl_fixed_t fy = wl_fixed_from_int(y - over_->getPosition().x);
    wl_pointer_send_motion(grab_->getWlResource(), getTime(), fx, fy);

    //surface enter, leave
}

void pointer::sendButtonPress(unsigned int button)
{
    if(!grab_)
        return;

    wl_pointer_send_button(grab_->getWlResource(), wl_display_next_serial(getWlDisplay()), getTime(), button, 1);

    //keyboard focus
}

void pointer::sendButtonRelease(unsigned int button)
{
    if(!grab_)
        return;

    wl_pointer_send_button(grab_->getWlResource(), wl_display_next_serial(getWlDisplay()), getTime(), button, 0);
}

void pointer::sendScroll(unsigned int axis, double value)
{
    if(!grab_)
        return;

    wl_pointer_send_axis(grab_->getWlResource(), getTime(), axis, value);
}

void pointer::setCursor(surfaceRes* surf, vec2ui hotspot)
{
    surf->setCursorRole(hotspot);
    cursor_ = surf;
}

//////////////////////////////////////
pointerRes::pointerRes(seatRes* sr, wl_client* client, unsigned int id) : resource(client, id, &wl_pointer_interface, &pointerImplementation), seatRes_(sr)
{
}

seat* pointerRes::getSeat() const
{
    return seatRes_->getSeat();
}

pointer* pointerRes::getPointer() const
{
    return getSeat()->getPointer();
}
