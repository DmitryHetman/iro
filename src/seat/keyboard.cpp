#include <seat/keyboard.hpp>

#include <seat/seat.hpp>
#include <wayland-server-protocol.h>

///////////////////////////////////////
void keyboardRelease(wl_client* client, wl_resource* resource)
{

}

const struct wl_keyboard_interface keyboardImplementation
{
    &keyboardRelease
};

///////////////////////////////////
keyboard::keyboard(seat& s) : seat_(s), grab_(nullptr)
{
}

keyboard::~keyboard()
{
}

void keyboard::sendKeyPress(unsigned int key)
{
    if(!grab_)
        return;

    wl_keyboard_send_key(&grab_->getWlResource(), wl_display_next_serial(iroWlDisplay()), getTime(), key, 1);
}

void keyboard::sendKeyRelease(unsigned int key)
{
    if(!grab_)
        return;

    wl_keyboard_send_key(&grab_->getWlResource(), wl_display_next_serial(iroWlDisplay()), getTime(), key, 0);
}

void keyboard::sendFocus(keyboardRes* newGrab)
{
    if(grab_)
    {
        //focus release
    }
    grab_ = newGrab;

    //focus gain
}

/////////////////////////
keyboardRes::keyboardRes(seatRes& sr, wl_client& client, unsigned int id) : resource(client, id, &wl_keyboard_interface, &keyboardImplementation), seatRes_(sr)
{

}

seat& keyboardRes::getSeat() const
{
    return seatRes_.getSeat();
}

keyboard& keyboardRes::getKeyboard() const
{
    return *getSeat().getKeyboard();
}
