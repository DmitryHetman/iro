#pragma once

#include <iro.hpp>
#include <resources/resource.hpp>

#include <wayland-server-core.h>

enum class pointerState
{
    normal,
    resize,
    move
};

class pointer : public nonCopyable
{
protected:
    friend seat;

    seat* seat_;
    pointerRes* grab_ = nullptr;
    pointerState state_ = pointerState::normal;

    surfaceRes* cursor_ = nullptr;

    pointer(seat* s);
public:
    void sendMove(unsigned int x, unsigned int y);
    void sendButtonPress(unsigned int button);
    void sendButtonRelease(unsigned int button);
    void sendScroll();

    bool hasGrab() const { return (grab_ == nullptr); }
    pointerRes* getGrab() const { return grab_; }

    pointerState getState() const { return state_; }
    seat* getSeat() const { return seat_; }

    void startResize();
    void startMove();
};
////////////////////////
class pointerRes : public resource
{
protected:
    friend seatRes;

    seatRes* seatRes_;
    pointerRes(seatRes* sr, wl_client* client, unsigned int id);

public:
    seatRes* getSeatRes() const { return seatRes_; }

    seat* getSeat() const;
    pointer* getPointer() const;

    resourceType getType() const { return resourceType::pointer; }
};
