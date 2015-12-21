#pragma once

#include <iro/include.hpp>
#include <nytl/nonCopyable.hpp>

#include <signal.h>
#include <string>

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

	int socks[2];
	int wlSocks[2];
	int xSocks[2];

	//really ugly to have here - has to include signal.h
	struct sigaction savedSignalHandler_;

protected:
	void openDisplay();
	void closeDisplay();
	void executeXWayland();

public:
	XWindowManager(Compositor& comp, Seat& seat);
	~XWindowManager();

	Compositor& compositor() const { return *compositor_; }
	Seat& seat() const { return *seat_; }

	void initWM();
};

}
