#pragma once

#include <iro.hpp>
#include <resources/resource.hpp>

#include <util/nonCopyable.hpp>

#include <wayland-server-core.h>

//////////////////////////////////////////
class keyboard : public nonCopyable
{
protected:
    friend seat;

    seat* seat_;
    keyboardRes* grab_ = nullptr;

    keyboard(seat* s);
public:
    void sendKeyPress(unsigned int key);
    void sendKeyRelease(unsigned int key);

    void sendFocus(keyboardRes* newGrab);

    bool hasGrab() const { return (grab_ == nullptr); }

    keyboardRes* getGrab() const { return grab_; }
};
//////////////////////////////////////////
class keyboardRes : public resource
{
protected:
    friend seatRes;

    seatRes* seatRes_;

    keyboardRes(seatRes* sr, wl_client* client, unsigned int id);
public:
    seatRes* getSeatRes() const { return seatRes_; }

    seat* getSeat() const;
    keyboard* getKeyboard() const;

    resourceType getType() const { return resourceType::keyboard; }
};
