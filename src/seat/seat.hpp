#pragma once

#include <iro.hpp>
#include <resources/resource.hpp>

////////////////////////////////
class seat
{
protected:
    pointer* pointer_;
    keyboard* keyboard_;

    shellSurfaceRes* grab_ = nullptr;

public:
    seat();
    ~seat();

    pointer* getPointer() const { return pointer_; }
    keyboard* getKeyboard() const { return keyboard_; }

    void resizeShellSurface(seatRes* res, shellSurfaceRes* shellSurf, unsigned int edges);
    void moveShellSurface(seatRes* res, shellSurfaceRes* shellSurf);

    shellSurfaceRes* getGrab() const { return grab_; }
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

    void unsetPointer();
    void unsetKeyboard();

    pointerRes* getPointerRes() const { return pointer_; }
    keyboardRes* getKeyboardRes() const { return keyboard_; }

    seat& getSeat() const { return seat_; }

    virtual resourceType getType() const { return resourceType::seat; }
};
