#pragma once

#include <iro/include.hpp>

#include <nytl/nonCopyable.hpp>
#include <nytl/time.hpp>

#include <xcb/xcb.h>

#include <string>
#include <memory>

namespace iro
{

///Responsible for setting up a x11 window manager and dealing with x11 clients.
class XWindowManager : public nytl::NonCopyable
{
protected:
	Compositor* compositor_ = nullptr;
	Seat* seat_ = nullptr;

	unsigned int display_ = 0;
	std::string displayName_ = "-";

	wl_client* screenClient_ = nullptr;
	wl_event_source* signalEventSource_ = nullptr;

	struct ListenerPOD;
	std::unique_ptr<ListenerPOD> destroyListener_;

	int socks[2] = {-1, -1};
	int wlSocks[2] = {-1, -1};
	int xSocks[2] = {-1, -1};

	nytl::TimePoint xwaylandStarted_;

	xcb_connection_t* xConnection_ = nullptr;
	xcb_screen_t* xScreen_ = nullptr;
	const xcb_query_extension_reply_t* xFixes_ = nullptr;
	xcb_window_t xWindow_ = 0;
   	xcb_window_t xFocus_ = 0;
	xcb_cursor_t xCursor_ = 0;

	xcb_atom_t xAtoms_[100]; 

	wl_event_source* xEventLoopSource_ = nullptr;

protected:
	static int sigusrHandler(int, void*);
	static void clientDestroyedHandler(wl_listener*, void*);
	static int xEventLoopHandler(int, unsigned int, void*);

protected:
	void openDisplay();
	void closeDisplay();
	void executeXWayland();
	void clientDestroyed();

	void initWM();
	void destroyWM();

	unsigned int xEventLoop();

public:
	XWindowManager(Compositor& comp, Seat& seat);
	~XWindowManager();

	Compositor& compositor() const { return *compositor_; }
	Seat& seat() const { return *seat_; }

};

}
