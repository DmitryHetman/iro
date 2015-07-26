#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>

#include <nyutil/nonCopyable.hpp>
#include <nyutil/vec.hpp>
#include <nyutil/callback.hpp>

class pointer : public nonCopyable
{
protected:
    friend seat; //manages pointer
    pointer(seat& s);
    ~pointer();

    seat& seat_;

    //grab_ contains the pointerRes for the over_ surface
    surfaceRes* over_ = nullptr;
    vec2i position_;

    //if this is == nullptr, the default cursor will be used
    surfaceRes* cursor_ = nullptr;

    callback<void(vec2i pos)> moveCallback_;
    callback<void(unsigned int button)> buttonPressCallback_;
    callback<void(unsigned int button)> buttonReleaseCallback_;
    callback<void(unsigned int axis, double value)> axisCallback_;

    callback<void(surfaceRes*, surfaceRes*)> focusCallback_;

    //helper func for sendMove
    void setActive(surfaceRes* res);

public:
    void sendMove(int x, int y);
    void sendMove(vec2i pos);

    void sendButtonPress(unsigned int button);
    void sendButtonRelease(unsigned int button);
    void sendAxis(unsigned int axis, double value);

    surfaceRes* getOver() const { return over_; }
    pointerRes* getActiveRes() const;

    seat& getSeat() const { return seat_; }

    void setCursor(surfaceRes& surf);
    void resetCursor();
    surfaceRes* getCursor() const { return cursor_; }

    vec2i getPosition() const { return position_; }
    vec2i getPositionWl() const;

    //cbs
    connection& onMove(std::function<void(vec2i)> func){ return moveCallback_.add(func); }
    connection& onButtonPress(std::function<void(unsigned int button)> func){ return buttonPressCallback_.add(func); }
    connection& onButtonRelease(std::function<void(unsigned int button)> func){ return buttonReleaseCallback_.add(func); }
    connection& onAxis(std::function<void(unsigned int axis, double value)> func){ return axisCallback_.add(func); }

    connection& onFocusChange(std::function<void(surfaceRes*, surfaceRes*)> func){ return focusCallback_.add(func); }
};
////////////////////////
class pointerRes : public resource
{
protected:
    friend seatRes;
    pointerRes(seatRes& sr, wl_client& client, unsigned int id);

    seatRes& seatRes_;

public:
    seatRes& getSeatRes() const { return seatRes_; }

    seat& getSeat() const;
    pointer& getPointer() const;

    virtual resourceType getType() const override { return resourceType::pointer; }
};
