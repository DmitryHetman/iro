#include <seat/seat.hpp>

#include <seat/keyboard.hpp>
#include <seat/pointer.hpp>

#include <compositor/compositor.hpp>

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
    new seatRes(getSeat(), client, id, version);
}

/////////////////////////
seat::seat()
{
    compositor* comp = getCompositor();
    if(!comp)
    {
        throw std::runtime_error("seat::seat(): no compositor initialized");
        return;
    }

    wl_global_create(comp->getWlDisplay(), &wl_seat_interface, 1, this, &bindSeat);

    keyboard_ = new keyboard(this);
    pointer_ = new pointer(this);
}

seat::~seat()
{
    delete keyboard_;
    delete pointer_;
}

//////////////////////////
seatRes::seatRes(seat* s, wl_client* client, unsigned int id, unsigned int version) : resource(client, id, &wl_seat_interface, &seatImplementation, version), seat_(s)
{
    wl_seat_send_capabilities(wlResource_, wl_seat_capability::WL_SEAT_CAPABILITY_KEYBOARD | wl_seat_capability::WL_SEAT_CAPABILITY_POINTER);
}
seatRes::~seatRes()
{
    if(pointer_) delete pointer_;
    if(keyboard_) delete keyboard_;
}

void seatRes::createPointer(unsigned int id)
{
    if(!pointer_)
    {
        pointer_ = new pointerRes(this, getWlClient(), id);
    }
    else
    {
        std::cout << "created 2nd pointer WARNING" << std::endl;
    }
}

void seatRes::createKeyboard(unsigned int id)
{
    if(!keyboard_)
    {
        keyboard_ = new keyboardRes(this, getWlClient(), id);
    }
    else
    {
        std::cout << "created 2nd keyboard WARNING" << std::endl;
    }
}

void seatRes::unsetPointer()
{
    pointer_ = nullptr;
}

void seatRes::unsetKeyboard()
{
    keyboard_ = nullptr;
}
