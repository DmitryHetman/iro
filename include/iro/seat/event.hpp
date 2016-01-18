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
namespace iro
{

namespace eventType
{
    constexpr unsigned char pointerButton = 0x01;
    constexpr unsigned char pointerFocus = 0x02;
    constexpr unsigned char keyboardKey = 0x03;
    constexpr unsigned char keyboardFocus = 0x04;
    constexpr unsigned char keyboardMods = 0x05;
    constexpr unsigned char ping = 0x06;
}

///The Event class represents a unique event that was sent to a client.
class Event
{
protected:
    Event(unsigned int t, Client* c = nullptr) : type(t), client(c) {}

public:
    const unsigned int type;
    Client* client;
	unsigned int serial = 0; //0: no serial
};

//pointer/////////////////////////////////////////
class PointerButtonEvent : public Event
{
public:
    PointerButtonEvent(bool pstate, unsigned int pbutton, Client* c = nullptr) 
		: Event(eventType::pointerButton, c), state(pstate), button(pbutton) {};

    bool state;
    unsigned int button;
};

class PointerFocusEvent : public Event
{
public:
    PointerFocusEvent(bool pstate, SurfaceRes* surf, Client* c = nullptr) 
		: Event(eventType::pointerFocus, c), state(pstate), surface(surf) {};

    bool state;
    SurfaceRes* surface;
};

//keyboard/////////////////////////////////////
class KeyboardKeyEvent : public Event
{
public:
    KeyboardKeyEvent(bool pstate, unsigned int pkey, Client* c = nullptr) 
		: Event(eventType::keyboardKey, c), state(pstate), key(pkey) {};

    bool state;
    unsigned int key;
};

class KeyboardFocusEvent : public Event
{
public:
    KeyboardFocusEvent(bool pstate, SurfaceRes* surf, Client* c = nullptr) 
		: Event(eventType::keyboardFocus, c),state(pstate), surface(surf) {};

    bool state;
    SurfaceRes* surface;
};

class KeyboardModsEvent : public Event
{
public:
    KeyboardModsEvent(Client* c = nullptr) 
		: Event(eventType::keyboardMods, c) {};
};

///Event sent to the client to test if it is alive
class PingEvent : public Event
{
public:
	PingEvent(Client* c = nullptr) : Event(eventType::ping, c) {}
};

}
