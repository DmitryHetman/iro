#pragma once

#include <iro.hpp>
#include <resources/resource.hpp>

#include <util/vec.hpp>

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
    surfaceRes* over_ = nullptr;

    pointerState state_ = pointerState::normal;
    union
    {
        unsigned int resizeEdges_ = 0;
    };

    vec2ui position_;

    surfaceRes* cursor_ = nullptr;

    pointer(seat* s);
    ~pointer();

public:
    void sendMove(unsigned int x, unsigned int y);
    void sendButtonPress(unsigned int button);
    void sendButtonRelease(unsigned int button);
    void sendScroll(unsigned int axis, double value);

    pointerRes* getGrab() const { return grab_; }

    pointerState getState() const { return state_; }
    seat* getSeat() const { return seat_; }

    void setCursor(surfaceRes* surf, vec2ui hotspot);
    surfaceRes* getCursor() const { return cursor_; }

    vec2ui getPosition() const { return position_; }
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
