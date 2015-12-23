#pragma once

#include <iro/include.hpp>
#include <nytl/nonCopyable.hpp>

#include <string>
#include <memory>

namespace iro
{

///Responsible for setting up a x11 window manager and dealing with x11 clients.
class XWindowManager : public nytl::nonCopyable
{
protected:
	Compositor* compositor_;
	Seat* seat_;

	unsigned int display_ = 0;
	std::string displayName_ = "-";

	wl_client* screenClient_ = nullptr;
	wl_event_source* signalEventSource_ = nullptr;

	struct ListenerPOD;
	std::unique_ptr<ListenerPOD> destroyListener_;

	int socks[2];
	int wlSocks[2];
	int xSocks[2];

protected:
	static int sigusrHandler(int, void*);
	static void clientDestroyedHandler(wl_listener*, void*);

protected:
	void openDisplay();
	void closeDisplay();
	void executeXWayland();

	void initWM();
	void clientDestroyed();

public:
	XWindowManager(Compositor& comp, Seat& seat);
	~XWindowManager();

	Compositor& compositor() const { return *compositor_; }
	Seat& seat() const { return *seat_; }

};

}
