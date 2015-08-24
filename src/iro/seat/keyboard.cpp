#include <iro/seat/keyboard.hpp>
#include <iro/seat/seat.hpp>
#include <iro/seat/event.hpp>
#include <iro/compositor/surface.hpp>
#include <iro/compositor/client.hpp>
#include <iro/iro.hpp>
#include <iro/util/log.hpp>

#include <wayland-server-protocol.h>

#include <linux/input.h>

///////////////////////////////////////
void keyboardRelease(wl_client* client, wl_resource* resource)
{
    keyboardRes* kb = (keyboardRes*) wl_resource_get_user_data(resource);
    kb->destroy();
}

const struct wl_keyboard_interface keyboardImplementation
{
    &keyboardRelease
};

///////////////////////////////////
keyboard::keyboard(seat& s) : seat_(s), focus_(nullptr)
{
}

keyboard::~keyboard()
{
}

void keyboard::sendKeyPress(unsigned int key)
{
    iroLog("Key ", key, " Pressed");

    if(key == KEY_ESC)
    {
        getIro()->exit();
        return;
    }

    if(!getActiveRes())
        return;

    keyboardKeyEvent* ev = new keyboardKeyEvent(1, key, &getActiveRes()->getClient());
    wl_keyboard_send_key(&getActiveRes()->getWlResource(), iroNextSerial(ev), iroTime(), key, 1);

    keyPressCallback_(key);
}

void keyboard::sendKeyRelease(unsigned int key)
{
    iroLog("Key ", key, " Released");

    if(!getActiveRes())
        return;

    keyboardKeyEvent* ev = new keyboardKeyEvent(0, key, &focus_->getClient());
    wl_keyboard_send_key(&getActiveRes()->getWlResource(), iroNextSerial(ev), iroTime(), key, 0);

    keyReleaseCallback_(key);
}

void keyboard::sendFocus(surfaceRes* newFocus)
{
    if(getActiveRes())
    {
        keyboardFocusEvent* ev = new keyboardFocusEvent(0, focus_, &focus_->getClient());
        wl_keyboard_send_leave(&getActiveRes()->getWlResource(), iroNextSerial(ev), &focus_->getWlResource());
    }

    focusCallback_(focus_, newFocus);
    focus_ = newFocus;

    if(getActiveRes())
    {
        keyboardFocusEvent* ev = new keyboardFocusEvent(1, focus_, &focus_->getClient());
        wl_keyboard_send_enter(&getActiveRes()->getWlResource(), iroNextSerial(ev), &focus_->getWlResource(), nullptr);
    }
}

keyboardRes* keyboard::getActiveRes() const
{
    return (focus_) ? focus_->getClient().getKeyboardRes() : nullptr;
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