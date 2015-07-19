#pragma once

#include <iro.hpp>
#include <resources/resource.hpp>

#include <util/nonCopyable.hpp>
#include <util/callback.hpp>

//////////////////////////////////////////
class keyboard : public nonCopyable
{
protected:
    friend seat; //manages keyboard
    keyboard(seat& s);
    ~keyboard();

    seat& seat_;
    surfaceRes* focus_ = nullptr;

    //callbacks
    callback<void(unsigned int)> keyPressCallback_;
    callback<void(unsigned int)> keyReleaseCallback_;

    callback<void(surfaceRes*, surfaceRes*)> focusCallback_;

public:
    void sendKeyPress(unsigned int key);
    void sendKeyRelease(unsigned int key);

    void sendFocus(surfaceRes* newFocus);
    surfaceRes* getFocus() const { return focus_; }
    keyboardRes* getActiveRes() const;

    seat& getSeat() const { return seat_; }

    //cbs
    connection& onKeyPress(std::function<void(unsigned int key)> func){ return keyPressCallback_.add(func); }
    connection& onKeyRelease(std::function<void(unsigned int key)> func){ return keyReleaseCallback_.add(func); }

    connection& onFocus(std::function<void(surfaceRes* old, surfaceRes*)> func){ return focusCallback_.add(func); }
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
    virtual resourceType getType() const override { return resourceType::keyboard; }
};
