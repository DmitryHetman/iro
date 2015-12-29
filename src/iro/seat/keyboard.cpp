#include <iro/seat/keyboard.hpp>

#include <iro/seat/seat.hpp>
#include <iro/seat/event.hpp>
#include <iro/compositor/surface.hpp>
#include <iro/compositor/client.hpp>
#include <iro/compositor/compositor.hpp>
#include <iro/util/os.hpp>

#include <nytl/log.hpp>
#include <nytl/make_unique.hpp>

#include <wayland-server-protocol.h>

#include <xkbcommon/xkbcommon.h>
#include <linux/input.h>

#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>

#include <cstring>


namespace iro
{

namespace
{

//wayland implementation
void keyboardRelease(wl_client*, wl_resource* resource)
{
	KeyboardRes* kbres = Resource::validateDisconnect<KeyboardRes>(resource, "keyboardRelease");
	if(!kbres) return;

    kbres->destroy();
}

const struct wl_keyboard_interface keyboardImplementation
{
    &keyboardRelease
};

}

//static c callback
int Keyboard::repeatTimerHandler(void* data)
{
	if(data) static_cast<Keyboard*>(data)->repeatTimerCallback();
	return 1;
}

//Keyboard implementation
Keyboard::Keyboard(Seat& seat) : seat_(&seat)
{
	struct xkb_rule_names rules;
	std::memset(&rules, 0, sizeof(rules));

	rules.rules = getenv("XKB_DEFAULT_RULES");
	rules.model = getenv("XKB_DEFAULT_MODEL");
    rules.layout = getenv("XKB_DEFAULT_LAYOUT");
	rules.variant = getenv("XKB_DEFAULT_VARIANT");
	rules.options = getenv("XKB_DEFAULT_OPTIONS");

	struct xkb_context* context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	if(!context)
	{
		nytl::sendWarning("Keyboard: failed to create xcb_context");
		return;
	}

	keymap_.xkb = xkb_map_new_from_names(context, &rules, XKB_KEYMAP_COMPILE_NO_FLAGS);
	if(!keymap_.xkb)
	{
		nytl::sendWarning("Keyboard: failed to create xkb keymap");
		return;
	}

	xkb_context_unref(context);
	context = nullptr;

	char* keymapStrC = xkb_map_get_as_string(keymap_.xkb);
	if(!keymapStrC)
	{
		nytl::sendWarning("Keyboard: failed to get xkb keymap string");
		return;
	}

	std::string keymapStr(keymapStrC);
	std::size_t mSize = keymapStr.size();

	keymap_.fd = os_create_anonymous_file(keymapStr.size());
	if(keymap_.fd < 0)
	{
		nytl::sendWarning("Keyboard: failed to open anonymous file for keymap mmap");
		return;
	}

	void* map = mmap(nullptr, mSize, PROT_READ | PROT_WRITE, MAP_SHARED, keymap_.fd, 0);	
	if(!map)
	{
		nytl::sendWarning("Keyboard: faield to mmap memory for keymap");
		return;
	}

	keymap_.mapped = static_cast<char*>(map);
	keymap_.mappedSize = mSize;

	memcpy(keymap_.mapped, keymapStr.c_str(), keymap_.mappedSize - 1);

	keymap_.state = xkb_state_new(keymap_.xkb);

	const char *modifierNames[modifierCount] =
	{
	   XKB_MOD_NAME_SHIFT,
	   XKB_MOD_NAME_CAPS,
	   XKB_MOD_NAME_CTRL,
	   XKB_MOD_NAME_ALT,
	   "Mod2",
	   "Mod3",
	   XKB_MOD_NAME_LOGO,
	   "Mod5",
	};

	for(unsigned int i = 0; i < modifierCount; ++i)
      keymap_.mods[i] = xkb_map_mod_get_index(keymap_.xkb, modifierNames[i]);

	//init repeat timer source
	repeat_.timer = wl_event_loop_add_timer(&compositor().wlEventLoop(), 
			&Keyboard::repeatTimerHandler, this);
}

Keyboard::~Keyboard()
{
	if(repeat_.timer)
	{
		wl_event_source_remove(repeat_.timer);
		repeat_.timer = nullptr;
	}

	if(keymap_.state)
	{
		xkb_state_unref(keymap_.state);
		keymap_.state = nullptr;
	}

	if(keymap_.xkb)
	{
		xkb_map_unref(keymap_.xkb);
		keymap_.xkb = nullptr;
	}

	if(keymap_.mapped)
	{
		munmap(keymap_.mapped, keymap_.mappedSize);
		keymap_.mappedSize = 0;
		keymap_.mapped = nullptr;
	}

	if(keymap_.fd >= 0)
	{
		close(keymap_.fd);
		keymap_.fd = -1;
	}
}

Compositor& Keyboard::compositor() const
{
	return seat().compositor();
}

void Keyboard::updateModifiers()
{
	unsigned int depressed = xkb_state_serialize_mods(keymap_.state, 
		static_cast<xkb_state_component>(XKB_STATE_DEPRESSED));
	unsigned int latched = xkb_state_serialize_mods(keymap_.state, 
			static_cast<xkb_state_component>(XKB_STATE_LATCHED));
	unsigned int locked = xkb_state_serialize_mods(keymap_.state, 
			static_cast<xkb_state_component>(XKB_STATE_LOCKED));
	unsigned int group = xkb_state_serialize_layout(keymap_.state, 
			static_cast<xkb_state_component>(XKB_STATE_LAYOUT_EFFECTIVE));

	if(mods_.depressed == depressed &&
		mods_.latched == latched &&
		mods_.locked == locked &&
		mods_.group == group)
      return;

	mods_.depressed = depressed;
	mods_.latched = latched;
	mods_.locked = locked;
	mods_.group = group;

	if(activeResource())
	{
		auto& ev = compositor().event(nytl::make_unique<KeyboardModsEvent>(), 1);
		wl_keyboard_send_modifiers(&activeResource()->wlResource(), ev.serial, 
			depressed, latched, locked, group);
	}

	modifiers_ = static_cast<Modifier>(modMask(depressed | latched));
	leds_ = static_cast<Led>(ledMask());
}

void Keyboard::sendKey(unsigned int key, bool press)
{
	nytl::sendLog("Key ", key, " ", press, " -- ", focus_, " -- ", activeResource());

	xkb_state_update_key(keymap_.state, key + 8, press ? XKB_KEY_DOWN : XKB_KEY_UP);
	bool previous = keys_[key];
	keys_[key] = press;

	if(previous && press)
	{
		//return;
	}

	//resetRepeat();
	updateModifiers();

	//check/init repeat
	if(press)
	{
		if(xkb_keymap_key_repeats(keymap_.xkb, key + 8))
		{
			//beginRepeat();
		}
	}

	//check grab
	if(grabbed_)
	{
		if(grab_.keyFunction)grab_.keyFunction(key, press);
		if(grab_.exclusive) return;
	}

	//exit on ESC key
	//only for debug - should NOT be in releases.
    if(key == KEY_ESC && press)
    {
		nytl::sendLog("ESC key - exiting compositor");
        compositor().exit();
        return;
    }

	//send key to client
    if(activeResource())
	{
		auto& ev = compositor().event(nytl::make_unique<KeyboardKeyEvent>(press, 
					key, &activeResource()->client()), 1);
		wl_keyboard_send_key(&activeResource()->wlResource(), ev.serial, 
			compositor().time(), key, press);
	}
	else
	{
		nytl::sendLog("key with no active resource...");
	}
     
	//callback	
    keyCallback_(key, press);
}

void Keyboard::wlPressedKeys(wl_array& arr)
{
	for(auto& k : keys_)
	{
		if(k.second)
		{
			uint32_t* arrayEntry = (uint32_t*) wl_array_add(&arr, sizeof(uint32_t));
			if(arrayEntry) *arrayEntry = k.first;
		}
	}
}

void Keyboard::sendFocus(SurfaceRes* newFocus)
{
    if(activeResource())
    {
        auto& ev = compositor().event(nytl::make_unique<KeyboardFocusEvent>(0, focus_.get(), 
				&focus_->client()), 1);
        wl_keyboard_send_leave(&activeResource()->wlResource(), ev.serial, &focus_->wlResource());
    }

	SurfaceRes* old = focus_.get();
    focus_.set(newFocus);

    if(activeResource())
    {
        auto& ev = compositor().event(nytl::make_unique<KeyboardFocusEvent>(1, focus_.get(), 
				&focus_->client()), 1);

		wl_array keys;
		wl_array_init(&keys);
		wlPressedKeys(keys);

        wl_keyboard_send_enter(&activeResource()->wlResource(), ev.serial, 
				&focus_->wlResource(), &keys);
    }

    focusCallback_(old, newFocus);
}

void Keyboard::beginRepeat()
{
	unsigned int delay = repeat_.repeating ? repeat_.rate : repeat_.delay;
	wl_event_source_timer_update(repeat_.timer, delay);

	repeat_.active = 0;
}

void Keyboard::resetRepeat()
{
	if(!repeat_.active) return;

	repeat_.repeating = 0;
	repeat_.active = 0;
	wl_event_source_timer_update(repeat_.timer, 0);
}

void Keyboard::repeatTimerCallback()
{
	wl_event_source_timer_update(repeat_.timer, 0);
	repeat_.active = 0;
	repeat_.repeating = 1;

	auto keysCopy = keys_;
	keys_.clear();

	for(auto& k : keysCopy)
	{
		if(!k.second) continue;
		if(xkb_keymap_key_repeats(keymap_.xkb, k.first + 8))
		{
			xkb_state_update_key(keymap_.state, k.first + 8, XKB_KEY_UP);
		}
		else
		{
			keys_[k.first] = 1;
		}
	}

	for(auto& k : keysCopy)
	{
		if(!k.second || !xkb_keymap_key_repeats(keymap_.xkb, k.first + 8)) continue;
		sendKey(k.first, 1);
	}
}

unsigned int Keyboard::modMask(unsigned int in) const
{
   uint32_t mods = 0;
   for (uint32_t i = 0; i < modifierCount; ++i) {
      if (keymap_.mods[i] != XKB_MOD_INVALID && (in & (1 << keymap_.mods[i])))
         mods |= (1 << i);
   }

   return mods;
}

unsigned int Keyboard::ledMask() const
{
   uint32_t leds = 0;
   for (uint32_t i = 0; i < ledCount; ++i) {
      if (xkb_state_led_index_is_active(keymap_.state, keymap_.leds[i]))
         leds |= (1 << i);
   }

   return leds;
}

KeyboardRes* Keyboard::activeResource() const
{
    return (focus_) ? focus_->client().keyboardResource() : nullptr;
}

bool Keyboard::grab(const Keyboard::Grab& grb, bool force)
{
	if(grabbed_)
	{
		if(!force) return 0;
		if(grab_.grabEndFunction)grab_.grabEndFunction(1);
	}

	grabbed_ = 1;
	grab_ = grb;
	return 1;
}

bool Keyboard::releaseGrab()
{
	if(grabbed_)
	{
		if(grab_.grabEndFunction)grab_.grabEndFunction(0);
		grabbed_ = 0;

		return 1;
	}

	return 0;
}

//Keyboard Resource
KeyboardRes::KeyboardRes(SeatRes& seatRes, unsigned int id)
	: Resource(seatRes.wlClient(), id, &wl_keyboard_interface, &keyboardImplementation, 
			seatRes.version()), seatRes_(&seatRes)
{
	wl_keyboard_send_keymap(&wlResource(), WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1, keyboard().keymapFd(),
		   	keyboard().keymapSize());

	
	if(version() >= WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION)
		wl_keyboard_send_repeat_info(&wlResource(), keyboard().repeatRate(), 
			keyboard().repeatDelay());
}

Keyboard& KeyboardRes::keyboard() const
{
	return *seat().keyboard();
}

Seat& KeyboardRes::seat() const
{
	return seatRes().seat();
}

}
