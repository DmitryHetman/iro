#include <iro/seat/pointer.hpp>

#include <iro/seat/seat.hpp>
#include <iro/seat/event.hpp>
#include <iro/compositor/surface.hpp>
#include <iro/compositor/client.hpp>
#include <iro/compositor/shellSurface.hpp>
#include <iro/backend/backend.hpp>
#include <iro/backend/output.hpp>

#include <iro/util/log.hpp>

#include <wayland-server-protocol.h>

//////////////////////////
void pointerSetCursor(wl_client* client, wl_resource* resource, unsigned int serial, wl_resource* surface, int hotspot_x, int hotspot_y)
{
}
void pointerRelease(wl_client* client, wl_resource* resource)
{
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

void pointer::setOver(surfaceRes* newOne)
{
    //send leave
    if(over_.get())
    {
        if(!over_.get()->getClient().getPointerRes())
            iroWarning("pointer::sendActive: ", "Left surface without associated pointerRes");

        else
        {
            pointerFocusEvent* ev = new pointerFocusEvent(0, over_.get());
            wl_pointer_send_leave(&over_.get()->getClient().getPointerRes()->getWlResource(), iroNextSerial(ev), &over_.get()->getWlResource());
        }
    }

    surfaceRes* old = over_.get();
    over_.set(newOne);

    //send enter
    if(newOne)
    {
        if(!newOne->getClient().getPointerRes())
            iroWarning("pointer::sendActive: ", "Entered surface without associated pointerRes");

        else
        {
            pointerFocusEvent* ev = new pointerFocusEvent(1, newOne);
            vec2i pos; //todo
            wl_pointer_send_enter(&newOne->getClient().getPointerRes()->getWlResource(), iroNextSerial(ev), &newOne->getWlResource(), pos.x, pos.y);
        }
    }

    focusCallback_(old, newOne);
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
    overOut->scheduleRepaint();

    if(seat_.getMode() == seatMode::normal)
    {
        surfaceRes* surf = overOut->getSurfaceAt(position_);
        if(surf != over_.get())
        {
            setOver(surf);
        }

        if(!getActiveRes())
            return;

        vec2i vec = getPositionWl();
        wl_pointer_send_motion(&getActiveRes()->getWlResource(), iroTime(), vec.x, vec.y);
    }

    else if(seat_.getMode() == seatMode::move)
    {
        if(!seat_.getGrab())
        {
            iroWarning("pointer::sendMove: ", "seat in move mode without any grab surface. Resetting");
            getSeat().cancelGrab();
        }
        else
        {
            getSeat().getGrab()->move(delta);
        }
    }

    else if(seat_.getMode() == seatMode::resize)
    {
        if(!seat_.getGrab())
        {
            iroWarning("pointer::sendMove: ", "seat in resize mode without any grab surface. Resetting");
            getSeat().cancelGrab();
        }
        else
        {
            //todo!
            if(!over_.get())
            {
                getSeat().cancelGrab();
            }

            int w = over_.get()->getExtents().width();
            int h = over_.get()->getExtents().height();

            if(!w || !h) return;

            unsigned int edges = getSeat().getResizeEdges();

            if(edges & WL_SHELL_SURFACE_RESIZE_BOTTOM)
                h = position_.y - over_.get()->getPosition().y;

            else if(edges & WL_SHELL_SURFACE_RESIZE_TOP)
                h = over_.get()->getExtents().bottom() - position_.y;

            if(edges & WL_SHELL_SURFACE_RESIZE_RIGHT)
                w = position_.x - over_.get()->getPosition().y;

            else if(edges & WL_SHELL_SURFACE_RESIZE_LEFT)
                w = over_.get()->getExtents().right() - position_.x;

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
    if(!getActiveRes() || getSeat().getMode() != seatMode::normal)
    {
        buttonPressCallback_(button);
        return;
    }

    pointerButtonEvent* ev = new pointerButtonEvent(1, button, &getActiveRes()->getClient());
    wl_pointer_send_button(&getActiveRes()->getWlResource(), iroNextSerial(ev), iroTime(), button, 1);

    buttonPressCallback_(button);
    //keyboard focus
}

void pointer::sendButtonRelease(unsigned int button)
{
    if(!getActiveRes() || getSeat().getMode() != seatMode::normal)
    {
        buttonReleaseCallback_(button);
        return;
    }

    pointerButtonEvent* ev = new pointerButtonEvent(0, button, &getActiveRes()->getClient());
    wl_pointer_send_button(&getActiveRes()->getWlResource(), iroNextSerial(ev), iroTime(), button, 0);

    buttonReleaseCallback_(button);
}

void pointer::sendAxis(unsigned int axis, double value)
{
    axisCallback_(axis, value);

    if(!getActiveRes() || getSeat().getMode() != seatMode::normal)
        return;

    wl_pointer_send_axis(&getActiveRes()->getWlResource(), iroTime(), axis, value);
}

void pointer::setCursor(surfaceRes& surf)
{
    if(surf.getRoleType() != surfaceRoleType::cursor)
    {
        iroWarning("pointer::setCursor: tried to set cursor to a surface without cursor role");
        return;
    }

    cursor_.set(&surf);
}

void pointer::resetCursor()
{
    cursor_.set(nullptr);
}

vec2i pointer::getPositionWl() const
{
    return vec2i(wl_fixed_from_int(position_.x), wl_fixed_from_int(position_.y));
}

pointerRes* pointer::getActiveRes() const
{
    return (over_.get()) ? over_.get()->getClient().getPointerRes() : nullptr;
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
