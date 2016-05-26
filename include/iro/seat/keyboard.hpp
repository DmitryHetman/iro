#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>

#include <nytl/nonCopyable.hpp>
#include <nytl/callback.hpp>
#include <nytl/observe.hpp>
#include <nytl/enumOps.hpp>

#include <functional>
#include <map>

//prototypes
struct xkb_keymap;
struct xkb_state;
struct wl_array;

namespace iro
{

///Represents a physical keyboard. Does automatically initialize a xkb keymap.
///The Keyboard can be implicitly grabbed by clients or the shell e.g. when a window is resized
///or a dnd-action takes place or the shell itself has Keyboard focus.
///Every extension and shell component can additionally register callbacks for keyboard events.
class Keyboard : public nytl::NonCopyable
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

public:
	Keyboard(Seat& seat);
	~Keyboard();

    void sendKey(unsigned int key, bool press, bool repeat = false);
    void sendFocus(SurfaceRes* newFocus);

    SurfaceRes* focus() const { return focus_.get(); }
    KeyboardRes* activeResource() const;

    Seat& seat() const { return *seat_; }
	Compositor& compositor() const;

	bool grab(const Grab& grb, bool force = 1);
	bool releaseGrab();

	void updateModifiers();
	void wlPressedKeys(wl_array& arr);

	nytl::Flags<Modifier> modifiers() const { return modifiers_; }
	Led leds() const { return leds_; }

	//get
	xkb_keymap& xkbKeymap() const { return *keymap_.xkb; }
	int keymapFd() const { return keymap_.fd; }
	std::size_t keymapSize() const { return keymap_.mappedSize; }

	unsigned int repeatDelay() const { return repeat_.delay; }
	unsigned int repeatRate() const { return repeat_.rate; }
	xkb_state& xkbState() const { return *keymap_.state; }

	unsigned int modMask(unsigned int in) const;
	unsigned int ledMask() const;

    //callbacks
	///The given function must have a signature compatible to void(unsigned int, bool, const char*).
	///The first paramter represents the linux keycode, the second if it was pressed or released
	///and the third is a null-terminated utf8 string of the given keycode.
    template<typename F> nytl::Connection onKey(F&& f)
		{ return keyCallback_.add(f); }

	///The given function must have a signature compatible to void(SurfaceRes*, SurfaceRes*).
	///The first parameter holds the old focused surface (or nullptr) and the second parameter
	///the new focused surface (or nullptr).
    template<typename F> nytl::Connection onFocus(F&& f)
		{ return focusCallback_.add(f); }

protected:
    Seat* seat_;
    SurfacePtr focus_;

	bool grabbed_ = 0;
	Grab grab_;

	std::map<unsigned int, bool> keys_; //all key states, first param is the linux key code
	nytl::Flags<Modifier> modifiers_; //all current modifiers
	Led leds_; //all current leds

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
		unsigned int delay = 600; //the delay in ms when the repeat should start
		unsigned int rate = 25; //the time in ms between key repeats
		unsigned int key = 0; //the current repeated key (or 0)
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
	nytl::Callback<void(unsigned int, bool, const char*)> keyCallback_;
	nytl::Callback<void(SurfaceRes*, SurfaceRes*)> focusCallback_;

protected:
	void beginRepeat(unsigned int key);
	void resetRepeat();
	void repeatTimerCallback();
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

NYTL_ENABLE_ENUM_OPS(iro::Keyboard::Modifier)
