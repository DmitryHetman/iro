#include <iro/seat/seat.hpp>

#include <iro/seat/keyboard.hpp>
#include <iro/seat/pointer.hpp>
#include <iro/seat/event.hpp>

#include <iro/util/log.hpp>

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
                                {
                                    std::cout << "release: " << button << " ev: " << ((pointerButtonEvent*)modeEvent_)->button << std::endl;
                                    if(modeEvent_ && modeEvent_->type == eventType::pointerButton && ((pointerButtonEvent*)modeEvent_)->button == button)
                                        cancelGrab();
                                }
                             });
}

seat::~seat()
{
    if(keyboard_)delete keyboard_;
    if(pointer_) delete pointer_;
}

void seat::moveShellSurface(unsigned int serial, seatRes* res, shellSurfaceRes* shellSurface)
{
    std::cout << "started move" << std::endl;

    event* ev = iroGetEvent(serial);
    if(!ev)
    {
        //error?
        return;
    }

    mode_ = seatMode::move;
    modeEvent_ = ev;
    grab_ = shellSurface;
}

void seat::resizeShellSurface(unsigned int serial, seatRes* res, shellSurfaceRes* shellSurface, unsigned int edges)
{
    std::cout << "started resize" << std::endl;

    event* ev = iroGetEvent(serial);
    if(!ev)
    {
        //error?
        return;
    }

    mode_ = seatMode::resize;
    modeEvent_ = ev;
    grab_ = shellSurface;
    resizeEdges_ = edges;
}

void seat::cancelGrab()
{
    std::cout << "cancelGrtab()" << std::endl;

    if(mode_ == seatMode::normal)
        iroWarning("seat::cancelGrab: ", "seat is already in normal mode");

    mode_ = seatMode::normal;
    modeEvent_ = nullptr;

    std::cout << "modeeee0: " << (int) mode_ << std::endl;
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
        //pointer_->onDestruct([=]{ pointer_ = nullptr; });
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
        //keyboard_->onDestruct([=]{ keyboard_ = nullptr; });
    }
    else
    {
        iroWarning("seatRes::createKeyboard: ", "tried to create second keyboardRes");
    }
}
