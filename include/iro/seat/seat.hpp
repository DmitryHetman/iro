#pragma once

#include <iro/include.hpp>
#include <iro/util/global.hpp>
#include <iro/compositor/resource.hpp>

#include <nytl/vec.hpp>

namespace iro
{

///The seat class represents a unix input connection. It can handle one pointer, keyboard or
///touch field.
class Seat : public Global
{
protected:
	Compositor* compositor_;
    std::string name_;

    std::unique_ptr<Pointer> pointer_;
    std::unique_ptr<Keyboard> keyboard_;
    std::unique_ptr<Touch> touch_;

public:
    Seat(Compositor& comp, const nytl::vec3b& capas = {1, 1, 1});
	~Seat();

	const std::string& name() const { return name_; }

    Pointer* pointer() const { return pointer_.get(); }
    Keyboard* keyboard() const { return keyboard_.get(); }
    Touch* touch() const { return touch_.get(); }

	Compositor& compositor() const { return *compositor_; }
};


///The SeatRes represents a clients resource for a seat global.
class SeatRes : public Resource
{
protected:
    Seat* seat_;

    PointerRes* pointer_ {nullptr};
    KeyboardRes* keyboard_ {nullptr};
    TouchRes* touch_ {nullptr};

public:
    SeatRes(Seat& s, wl_client& client, unsigned int id, unsigned int version);
    ~SeatRes();

    bool createPointer(unsigned int id);
    bool createKeyboard(unsigned int id);
    bool createTouch(unsigned int id);

    PointerRes* pointerResource() const { return pointer_; }
    KeyboardRes* keyboardResource() const { return keyboard_; }
    TouchRes* touchResource() const { return touch_; }

    Seat& seat() const { return *seat_; }

    virtual unsigned int type() const override { return resourceType::seat; }
};

}
