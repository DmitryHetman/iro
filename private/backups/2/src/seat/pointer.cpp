#include <seat/pointer.hpp>

#include <seat/seat.hpp>

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
pointer::pointer(seat* s) : seat_(s), grab_(nullptr)
{
}

void pointer::sendMove(unsigned int x, unsigned int y)
{

}

void pointer::sendButtonPress(unsigned int button)
{

}

void pointer::sendButtonRelease(unsigned int button)
{

}

void pointer::sendScroll()
{

}

void pointer::startResize()
{

}

void pointer::startMove()
{

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
