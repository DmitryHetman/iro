#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>

////////////////////////////////
enum class seatMode : unsigned char
{
    normal,
    resize,
    move,
    dnd
};

class seat
{
protected:
    std::string name_;

    pointer* pointer_;
    keyboard* keyboard_;

    event* modeEvent_ = nullptr; //event that brand seat in current state, if normal state its == nullptr
    seatMode mode_ = seatMode::normal;
    union
    {
        struct { shellSurfaceRes* grab_; unsigned int resizeEdges_ = 0; }; //resize and resize
        struct { }; //dnd
    };

public:
    seat();
    ~seat();

    pointer* getPointer() const { return pointer_; }
    keyboard* getKeyboard() const { return keyboard_; }

    void cancelGrab();
    void resizeShellSurface(unsigned int serial, seatRes* res, shellSurfaceRes* shellSurf, unsigned int edges);
    void moveShellSurface(unsigned int serial, seatRes* res, shellSurfaceRes* shellSurf);

    seatMode getMode() const { return mode_; }
    shellSurfaceRes* getGrab() const { return grab_; }
    unsigned int getResizeEdges() const { return (mode_ == seatMode::resize) ? resizeEdges_ : 0; }
};

////////////////////////////////
class seatRes : public resource
{
protected:
    seat& seat_;

    pointerRes* pointer_ = nullptr;
    keyboardRes* keyboard_ = nullptr;

public:
    seatRes(seat& s, wl_client& client, unsigned int id, unsigned int version);
    ~seatRes();

    void createPointer(unsigned int id);
    void createKeyboard(unsigned int id);

    pointerRes* getPointerRes() const { return pointer_; }
    keyboardRes* getKeyboardRes() const { return keyboard_; }

    seat& getSeat() const { return seat_; }

    //res
    virtual resourceType getType() const override { return resourceType::seat; }
};
