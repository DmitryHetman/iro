#pragma once

#include <iro.hpp>
#include <resources/resource.hpp>

#include <util/nonCopyable.hpp>

//////////////////////////////////////////
class keyboard : public nonCopyable
{
protected:
    friend seat;

    seat& seat_;
    keyboardRes* grab_ = nullptr;

    keyboard(seat& s);
    ~keyboard();

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
    keyboardRes(seatRes& sr, wl_client& client, unsigned int id);

    seatRes& seatRes_;

public:
    seatRes& getSeatRes() const { return seatRes_; }

    seat& getSeat() const;
    keyboard& getKeyboard() const;

    //res
    resourceType getType() const { return resourceType::keyboard; }
};
