#include <iro/seat/pointer.hpp>

#include <iro/seat/seat.hpp>
#include <iro/seat/event.hpp>
#include <iro/resources/surface.hpp>
#include <iro/resources/client.hpp>
#include <iro/resources/shellSurface.hpp>
#include <iro/backend/backend.hpp>
#include <iro/backend/output.hpp>

#include <iro/log.hpp>

#include <wayland-server-protocol.h>

//////////////////////////
void pointerSetCursor(wl_client* client, wl_resource* resource, unsigned int serial, wl_resource* surface, int hotspot_x, int hotspot_y)
{
    pointerRes* res = (pointerRes*) wl_resource_get_user_data(resource);
    pointer& p = res->getPointer();

    if(p.getActiveRes() != res)
        return;

    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(surface);
    surf->setCursor(vec2i(hotspot_x,hotspot_y));

    p.setCursor(*surf);
}
void pointerRelease(wl_client* client, wl_resource* resource)
{
    pointerRes* pt = (pointerRes*) wl_resource_get_user_data(resource);
    pt->destroy();
}

const struct wl_pointer_interface pointerImplementation
{
    &pointerSetCursor,
    &pointerRelease
};

////////////////////////////////////777
pointer::pointer(seat& s) : seat_(s)
{
}

pointer::~pointer()
{
}

void pointer::setActive(surfaceRes* surf)
{
    //send leave
    if(over_)
    {
        if(!over_->getClient().getPointerRes())
            iroWarning("pointer::sendActive: ", "Left surface without associated pointerRes");

        else
        {
            pointerFocusEvent* ev = new pointerFocusEvent(0, over_);
            wl_pointer_send_leave(&over_->getClient().getPointerRes()->getWlResource(), iroNextSerial(), &over_->getWlResource());
            iroRegisterEvent(*ev);
        }
    }

    //send enter
    if(surf)
    {
        if(!surf->getClient().getPointerRes())
            iroWarning("pointer::sendActive: ", "Entered surface without associated pointerRes");

        else
        {
            pointerFocusEvent* ev = new pointerFocusEvent(1, surf);
            wl_pointer_send_leave(&surf->getClient().getPointerRes()->getWlResource(), iroNextSerial(), &surf->getWlResource());
            iroRegisterEvent(*ev);
        }
    }

    focusCallback_(over_, surf);
    over_ = surf;
}

void pointer::sendMove(vec2i pos)
{
    vec2i delta = pos - position_;
    position_ = pos;

    moveCallback_(pos);

    /* TODO
    output* overOut = outputAt(x,y);
    if(!overOut)return;
    */

    output* overOut = iroBackend()->getOutputs()[0];
    overOut->refresh();

    if(getSeat().getMode() == seatMode::normal)
    {
        surfaceRes* surf = overOut->getSurfaceAt(position_);
        if(surf != over_)
        {
            setActive(surf);
        }

        if(!getActiveRes())
            return;

        auto v = getPositionWl();
        wl_pointer_send_motion(&getActiveRes()->getWlResource(), iroTime(), v.x, v.y);
    }

    else if(getSeat().getMode() == seatMode::move)
    {
        if(!getSeat().getGrab())
        {
            iroWarning("pointer::sendMove: ", "seat in move mode without any grab surface. Resetting");
            getSeat().cancelGrab();
        }
        else
        {
            getSeat().getGrab()->move(delta);
        }
    }

    else if(getSeat().getMode() == seatMode::resize)
    {
        if(!getSeat().getGrab())
        {
            iroWarning("pointer::sendMove: ", "seat in resize mode without any grab surface. Resetting");
            getSeat().cancelGrab();
        }
        else
        {
            int w = 0, h = 0;
            unsigned int edges = getSeat().getResizeEdges();

            if(edges & WL_SHELL_SURFACE_RESIZE_BOTTOM)
                h = position_.y - over_->getPosition().y;

            else if(edges & WL_SHELL_SURFACE_RESIZE_TOP)
                h = over_->getExtents().bottom() - position_.y;

            if(edges & WL_SHELL_SURFACE_RESIZE_RIGHT)
                w = position_.x - over_->getPosition().y;

            else if(edges & WL_SHELL_SURFACE_RESIZE_LEFT)
                w = over_->getExtents().right() - position_.x;

            if(w < 0) w = 0;
            if(h < 0) h = 0;

            wl_shell_surface_send_configure(&getSeat().getGrab()->getWlResource(), getSeat().getResizeEdges(), w, h);
        }
    }
}

void pointer::sendMove(int x, int y)
{
   sendMove(vec2i(x,y));
}

void pointer::sendButtonPress(unsigned int button)
{
    if(!getActiveRes())
        return;

    pointerButtonEvent* ev = new pointerButtonEvent(1, button, &getActiveRes()->getClient());
    wl_pointer_send_button(&getActiveRes()->getWlResource(), iroNextSerial(), iroTime(), button, 1);
    iroRegisterEvent(*ev);

    buttonPressCallback_(button);

    //keyboard focus
}

void pointer::sendButtonRelease(unsigned int button)
{
    if(!getActiveRes())
        return;

    pointerButtonEvent* ev = new pointerButtonEvent(0, button, &getActiveRes()->getClient());
    wl_pointer_send_button(&getActiveRes()->getWlResource(), iroNextSerial(), iroTime(), button, 0);
    iroRegisterEvent(*ev);

    buttonReleaseCallback_(button);
}

void pointer::sendAxis(unsigned int axis, double value)
{
    if(!getActiveRes())
        return;

    wl_pointer_send_axis(&getActiveRes()->getWlResource(), iroTime(), axis, value);
    axisCallback_(axis, value);
}

void pointer::setCursor(surfaceRes& surf)
{
    if(surf.getRole() != surfaceRole::cursor)
    {
        iroWarning("pointer::setCursor: tried to set cursor to a surface without cursor role");
    }

    cursor_ = &surf;
}

void pointer::resetCursor()
{
    cursor_ = nullptr;
}

vec2i pointer::getPositionWl() const
{
    return vec2i(wl_fixed_from_int(position_.x), wl_fixed_from_int(position_.y));
}

pointerRes* pointer::getActiveRes() const
{
    return (over_) ? over_->getClient().getPointerRes() : nullptr;
}

//////////////////////////////////////
pointerRes::pointerRes(seatRes& sr, wl_client& client, unsigned int id) : resource(client, id, &wl_pointer_interface, &pointerImplementation), seatRes_(sr)
{
}

seat& pointerRes::getSeat() const
{
    return seatRes_.getSeat();
}

pointer& pointerRes::getPointer() const
{
    return *getSeat().getPointer();
}
