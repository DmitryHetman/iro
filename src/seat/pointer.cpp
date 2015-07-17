#include <seat/pointer.hpp>

#include <seat/seat.hpp>
#include <resources/surface.hpp>
#include <resources/client.hpp>
#include <resources/shellSurface.hpp>
#include <backend/backend.hpp>
#include <backend/output.hpp>

#include <wayland-server-protocol.h>

//////////////////////////
void pointerSetCursor(wl_client* client, wl_resource* resource, unsigned int serial, wl_resource* surface, int hotspot_x, int hotspot_y)
{
    pointerRes* res = (pointerRes*) wl_resource_get_user_data(resource);
    pointer& p = res->getPointer();

    if(p.getGrab() != res)
        return;

    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(surface);
    p.setCursor(surf, vec2ui(hotspot_x, hotspot_y));
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
pointer::pointer(seat& s) : seat_(s), grab_(nullptr)
{
}

pointer::~pointer()
{

}

void pointer::sendMove(int x, int y)
{
    vec2i delta = vec2ui(x,y) - position_;
    position_.x = x;
    position_.y = y;

/*
    output* overOut = outputAt(x,y);
    if(!overOut)return;
        */

    output* overOut = iroBackend()->getOutputs()[0];

    overOut->refresh();


    if(state_ == pointerState::normal)
    {
        //surface enter, leave
        surfaceRes* surf = overOut->getSurfaceAt(position_);
        if(surf != over_)
        {
            grab_ = nullptr;
            if(over_)
            {
                if(!over_->getClient().iroSeatRes())
                {
                    std::cout << "w1" << std::endl;
                    return;
                }

                pointerRes* pres = over_->getClient().iroSeatRes()->getPointerRes();
                wl_pointer_send_leave(&pres->getWlResource(), wl_display_next_serial(iroWlDisplay()), &over_->getWlResource());
            }
            if(surf)
            {
                if(!surf->getClient().iroSeatRes())
                {
                    std::cout << "w2" << std::endl;
                    return;
                }

                wl_fixed_t fx = wl_fixed_from_int(x - surf->getPosition().x);
                wl_fixed_t fy = wl_fixed_from_int(y - surf->getPosition().x);

                pointerRes* pres = surf->getClient().iroSeatRes()->getPointerRes();
                wl_pointer_send_enter(&pres->getWlResource(), wl_display_next_serial(iroWlDisplay()), &surf->getWlResource(), fx, fy);

                grab_ = pres;
            }
            over_ = surf;
        }

        if(!grab_ || !over_)
            return;

        wl_fixed_t fx = wl_fixed_from_int(x - over_->getPosition().x);
        wl_fixed_t fy = wl_fixed_from_int(y - over_->getPosition().x);
        wl_pointer_send_motion(&grab_->getWlResource(), getTime(), fx, fy);
    }
    else if(state_ == pointerState::move)
    {
        iroSeat().getGrab()->move(delta);
    }
    else if(state_ == pointerState::resize)
    {
        int w = 0, h = 0;

        if(resizeEdges_ & WL_SHELL_SURFACE_RESIZE_BOTTOM)
            h = position_.y - over_->getPosition().y;
        else if(resizeEdges_ & WL_SHELL_SURFACE_RESIZE_TOP)
            h = over_->getExtents().bottom() - position_.y;

        if(resizeEdges_ & WL_SHELL_SURFACE_RESIZE_RIGHT)
            w = position_.x - over_->getPosition().y;
        else if(resizeEdges_ & WL_SHELL_SURFACE_RESIZE_LEFT)
            w = over_->getExtents().right() - position_.x;

        wl_shell_surface_send_configure(&iroSeat().getGrab()->getWlResource(), resizeEdges_, w, h);
    }
}

void pointer::sendButtonPress(unsigned int button)
{
    if(!grab_)
        return;

    wl_pointer_send_button(&grab_->getWlResource(), wl_display_next_serial(iroWlDisplay()), getTime(), button, 1);

    //keyboard focus
}

void pointer::sendButtonRelease(unsigned int button)
{
    if(!grab_)
        return;

    wl_pointer_send_button(&grab_->getWlResource(), wl_display_next_serial(iroWlDisplay()), getTime(), button, 0);
}

void pointer::sendScroll(unsigned int axis, double value)
{
    if(!grab_)
        return;

    wl_pointer_send_axis(&grab_->getWlResource(), getTime(), axis, value);
}

void pointer::setCursor(surfaceRes* surf, vec2ui hotspot)
{
    surf->setCursorRole(hotspot);
    cursor_ = surf;
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
