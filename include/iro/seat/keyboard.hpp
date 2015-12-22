#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>

#include <nytl/nonCopyable.hpp>
#include <nytl/callback.hpp>
#include <nytl/watchable.hpp>

#include <functional>

//prototypes
struct xkb_keymap;
struct xkb_state;

namespace iro
{

///Represnts a physical keyboard
class Keyboard : public nytl::nonCopyable
{
public:
	struct Grab
	{
		bool exclusive;
		std::function<void(unsigned int, bool)> keyFunction;
		//std::function<void(SurfaceRes*, SurfaceRes*)> focusFunction;
		std::function<void(bool)> grabEndFunction; //paremeter === new grabber
	};

protected:
    Seat* seat_;
    SurfaceRef focus_;

	bool grabbed_ = 0;
	Grab grab_;

	struct
	{
		unsigned int depressed = 0;
		unsigned int latched = 0;
		unsigned int locked = 0;
		unsigned int group = 0;
	} mods_;

	struct
	{
		wl_event_source* timer = nullptr;
		unsigned int delay = 1000;
		unsigned int repeatTime = 50;
	} repeat_;

	struct
	{
		xkb_keymap* xkb = nullptr;
		int fd = -1;
		std::size_t mappedSize = 0;
		char* mapped = nullptr;

		xkb_state* state = nullptr;
	} keymap_;

    //callbacks
	nytl::callback<void(unsigned int, bool)> keyCallback_;
	nytl::callback<void(SurfaceRes*, SurfaceRes*)> focusCallback_;

public:
	Keyboard(Seat& seat);
	~Keyboard();

    void sendKey(unsigned int key, bool press);
    void sendFocus(SurfaceRes* newFocus);

    SurfaceRes* focus() const { return focus_.get(); }
    KeyboardRes* activeResource() const;

    Seat& seat() const { return *seat_; }
	Compositor& compositor() const;

	bool grab(const Grab& grb, bool force = 1);
	bool releaseGrab();

	//get
	int keymapFd() const { return keymap_.fd; }
	std::size_t keymapSize() const { return keymap_.mappedSize; }

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
	SeatRes* seatRes_ = nullptr;

public:
	KeyboardRes(SeatRes& seatRes, unsigned int id);
	~KeyboardRes() = default;

    SeatRes& seatRes() const { return *seatRes_; }
    Keyboard& keyboard() const;
    Seat& seat() const;

    virtual unsigned int type() const override { return resourceType::keyboard; }
};

}
