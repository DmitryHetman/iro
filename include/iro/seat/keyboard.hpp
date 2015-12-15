#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>

#include <nytl/nonCopyable.hpp>
#include <nytl/callback.hpp>

namespace iro
{

///Represnts a physical keyboard
class Keyboard : public nytl::nonCopyable
{
protected:
    Seat* seat_;
    SurfaceRes* focus_ = nullptr;

    //callbacks
	nytl::callback<void(unsigned int, bool)> keyCallback_;
	nytl::callback<void(SurfaceRes*, SurfaceRes*)> focusCallback_;

public:
	Keyboard(Seat& seat);
	~Keyboard();

    void sendKey(unsigned int key, bool press);
    void sendFocus(SurfaceRes* newFocus);

    SurfaceRes* focus() const { return focus_; }
    KeyboardRes* activeResource() const;

    Seat& seat() const { return *seat_; }
	Compositor& compositor() const;

    //callbacks
    template<typename F> nytl::connection onKey(F&& f)
		{ return keyCallback_.add(f); }
    template<typename F> nytl::connection onFocus(F&& f)
		{ return focusCallback_.add(f); }
};

///Represents a clients keyboard resource.
class KeyboardRes : public Resource
{
protected:
	SeatRes* seatRes_;

public:
	KeyboardRes(SeatRes& seatRes, unsigned int id);
	~KeyboardRes() = default;

    SeatRes& seatRes() const { return *seatRes_; }
    Keyboard& keyboard() const;
    Seat& seat() const;

    virtual unsigned int type() const override { return resourceType::keyboard; }
};

}
