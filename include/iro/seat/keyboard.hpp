#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>

#include <nytl/nonCopyable.hpp>
#include <nytl/callback.hpp>
#include <nytl/watchable.hpp>

#include <functional>
#include <map>

//prototypes
struct xkb_keymap;
struct xkb_state;
struct wl_array;

namespace iro
{

///Represents a physical keyboard. Does automatically initialize a xbk keymap.
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

	enum class Modifier
	{
		shift = (1 << 0),
		caps = (1 << 1),
		ctrl = (1 << 2),
		alt = (1 << 3),
		mod2 = (1 << 4),
		mod3 = (1 << 5),
		logo = (1 << 6),
		mod5 = (1 << 7)
	};
	static constexpr unsigned int modifierCount = 8;

	enum class Led
	{
		num = (1 << 0),
		caps = (1 << 1),
		scroll = (1 << 2)
	};
	static constexpr unsigned int ledCount = 3;

	static int repeatTimerHandler(void*);

protected:
    Seat* seat_;
    SurfaceRef focus_;

	bool grabbed_ = 0;
	Grab grab_;

	std::map<unsigned int, bool> keys_; 
	Modifier modifiers_;
	Led leds_;

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
		unsigned int delay = 660;
		unsigned int rate = 25;
		bool repeating = 0;
		bool active = 0;
	} repeat_;

	struct
	{
		xkb_keymap* xkb = nullptr;
		int fd = -1;
		std::size_t mappedSize = 0;
		char* mapped = nullptr;
		xkb_state* state = nullptr;

		unsigned int mods[modifierCount];
		unsigned int leds[ledCount];
	} keymap_;

    //callbacks
	nytl::callback<void(unsigned int, bool)> keyCallback_;
	nytl::callback<void(SurfaceRes*, SurfaceRes*)> focusCallback_;

protected:
	void beginRepeat();
	void resetRepeat();
	void repeatTimerCallback();

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

	void updateModifiers();
	void wlPressedKeys(wl_array& arr);

	Modifier modifiers() const { return modifiers_; }
	Led leds() const { return leds_; }

	//get
	xkb_keymap* xkbKeymap() const { return keymap_.xkb; }
	int keymapFd() const { return keymap_.fd; }
	std::size_t keymapSize() const { return keymap_.mappedSize; }

	unsigned int repeatDelay() const { return repeat_.delay; }
	unsigned int repeatRate() const { return repeat_.rate; }
	xkb_state* xkbState() const { return keymap_.state; }

	unsigned int modMask(unsigned int in) const;
	unsigned int ledMask() const;

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
