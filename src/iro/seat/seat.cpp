#include <iro/seat/seat.hpp>

#include <iro/seat/keyboard.hpp>
#include <iro/seat/pointer.hpp>

#include <iro/log.hpp>

#include <wayland-server-protocol.h>

#include <stdexcept>
#include <iostream>

void seatGetPointer(wl_client* client, wl_resource* resource, unsigned int id)
{
    seatRes* res = (seatRes*) wl_resource_get_user_data(resource);
    res->createPointer(id);
}
void seatGetKeyboard(wl_client* client, wl_resource* resource, unsigned int id)
{
    seatRes* res = (seatRes*) wl_resource_get_user_data(resource);
    res->createKeyboard(id);
}
void seatGetTouch(wl_client* client, wl_resource* resource, unsigned int id)
{
}

const struct wl_seat_interface seatImplementation
{
    &seatGetPointer,
    &seatGetKeyboard,
    &seatGetTouch
};

void bindSeat(wl_client* client, void* data, unsigned int version, unsigned int id)
{
    new seatRes(*iroSeat(), *client, id, version);
}

/////////////////////////
seat::seat()
{
    wl_global_create(iroWlDisplay(), &wl_seat_interface, 1, this, &bindSeat);

    keyboard_ = new keyboard(*this);
    pointer_ = new pointer(*this);

    pointer_->onButtonRelease([=](unsigned int button){
                                if(mode_ != seatMode::normal)
                                    cancelGrab();
                             });
}

seat::~seat()
{
    if(keyboard_)delete keyboard_;
    if(pointer_) delete pointer_;
}

void seat::moveShellSurface(seatRes* res, shellSurfaceRes* shellSurface)
{
    mode_ = seatMode::move;
    grab_ = shellSurface;
}

void seat::resizeShellSurface(seatRes* res, shellSurfaceRes* shellSurface, unsigned int edges)
{
    mode_ = seatMode::resize;

    grab_ = shellSurface;
    resizeEdges_ = edges;
}

void seat::cancelGrab()
{
    if(mode_ == seatMode::normal)
        iroWarning("seat::cancelGrab: ", "seat is already in normal mode");

    mode_ = seatMode::normal;
}

//////////////////////////
seatRes::seatRes(seat& s, wl_client& client, unsigned int id, unsigned int version) : resource(client, id, &wl_seat_interface, &seatImplementation, version), seat_(s)
{
    wl_seat_send_capabilities(wlResource_, wl_seat_capability::WL_SEAT_CAPABILITY_KEYBOARD | wl_seat_capability::WL_SEAT_CAPABILITY_POINTER);
}
seatRes::~seatRes()
{
    if(pointer_) pointer_->destroy();
    if(keyboard_) keyboard_->destroy();
}

void seatRes::createPointer(unsigned int id)
{
    if(!pointer_)
    {
        pointer_ = new pointerRes(*this, getWlClient(), id);
        pointer_->onDestruct([=]{ pointer_ = nullptr; });
    }
    else
    {
        iroWarning("seatRes::createPointer: ", "tried to create second pointerRes");
    }
}

void seatRes::createKeyboard(unsigned int id)
{
    if(!keyboard_)
    {
        keyboard_ = new keyboardRes(*this, getWlClient(), id);
        keyboard_->onDestruct([=]{ keyboard_ = nullptr; });
    }
    else
    {
        iroWarning("seatRes::createKeyboard: ", "tried to create second keyboardRes");
    }
}
