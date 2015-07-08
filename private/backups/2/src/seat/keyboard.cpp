#include <seat/keyboard.hpp>

#include <seat/seat.hpp>

///////////////////////////////////////
void keyboardRelease(wl_client* client, wl_resource* resource)
{

}

const struct wl_keyboard_interface keyboardImplementation
{
    &keyboardRelease
};

///////////////////////////////////
keyboard::keyboard(seat* s) : seat_(s), grab_(nullptr)
{
}

void keyboard::sendKeyPress(unsigned int key)
{

}

void keyboard::sendKeyRelease(unsigned int key)
{

}

void keyboard::sendFocus(keyboardRes* newGrab)
{

}

/////////////////////////
keyboardRes::keyboardRes(seatRes* sr, wl_client* client, unsigned int id) : resource(client, id, &wl_keyboard_interface, &keyboardImplementation), seatRes_(sr)
{

}

seat* keyboardRes::getSeat() const
{
    return seatRes_->getSeat();
}

keyboard* keyboardRes::getKeyboard() const
{
    return getSeat()->getKeyboard();
}
