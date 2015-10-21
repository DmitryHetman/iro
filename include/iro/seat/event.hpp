#pragma once

#include <iro/include.hpp>

/* The compositor has to keep track of the sent events because
 * some requests from clients can only be done as an reaction to
 * client input, to check if this input is "a valid reason" for the
 * request it has to know which event is associated with the serial.
 * Furthermore, for some requests it can be important to know the
 * context,e.g. when a shellSurface is moved, which button was pressed
 * to start that -> at which button release stop moving the shellSurface
 */

namespace eventType
{
    constexpr unsigned char pointerButton = 0x01;
    constexpr unsigned char pointerFocus = 0x02;
    constexpr unsigned char keyboardKey = 0x03;
    constexpr unsigned char keyboardFocus = 0x04;
}

//base///////////////////////////////////////////
class event
{
protected:
    event(unsigned int t, client* c = nullptr) : type(t) {}

public:
    const unsigned int type;
    client* target;
};

//pointer/////////////////////////////////////////
class pointerButtonEvent : public event
{
public:
    pointerButtonEvent(bool pstate, unsigned int pbutton, client* c = nullptr) : event(eventType::pointerButton, c),
                                                                                    state(pstate), button(pbutton) {};
    bool state;
    unsigned int button;
};

class pointerFocusEvent : public event
{
public:
    pointerFocusEvent(bool pstate, surfaceRes* surf, client* c = nullptr) : event(eventType::pointerFocus, c),
                                                                                state(pstate), surface(surf) {};
    bool state;
    surfaceRes* surface;
};

//keyboard/////////////////////////////////////
class keyboardKeyEvent : public event
{
public:
    keyboardKeyEvent(bool pstate, unsigned int pkey, client* c = nullptr) : event(eventType::keyboardKey, c),
                                                                                state(pstate), key(pkey) {};
    bool state;
    unsigned int key;
};

class keyboardFocusEvent : public event
{
public:
    keyboardFocusEvent(bool pstate, surfaceRes* surf, client* c = nullptr) : event(eventType::keyboardFocus, c),
                                                                                state(pstate), surface(surf) {};
    bool state;
    surfaceRes* surface;
};
