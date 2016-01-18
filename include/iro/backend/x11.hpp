
#pragma once

#include <iro/include.hpp>
#include <iro/backend/backend.hpp>
#include <iro/backend/output.hpp>

#include <xcb/xcb.h>
#include <X11/Xlib.h>

namespace iro
{

class X11Output;

///Implements the Backend class for a x11 environment.
class X11Backend : public Backend
{
public:
	static bool available();

private:
	static int eventCallback(int, unsigned int, void*);

protected:
	Display* xDisplay_ = nullptr;
	xcb_connection_t* xConnection_ = nullptr;
	xcb_screen_t* xScreen_ = nullptr;

	wl_event_source* inputEventSource_ = nullptr;

	std::unique_ptr<WaylandEglContext> eglContext_;
	Compositor* compositor_;
	Seat* seat_;

	unsigned char xkbEventBase_ = 0;

	void xkbSetup();
	int eventLoop();
	X11Output* outputForWindow(const xcb_window_t& win);

public:
	X11Backend(Compositor& comp, Seat& seat);
	virtual ~X11Backend();

	Display* xDisplay() const { return xDisplay_; }
    xcb_connection_t* xConnection() const { return xConnection_; }
    xcb_screen_t* xScreen() const { return xScreen_; }

	Compositor& compositor() const { return *compositor_; }
	Seat& seat() const { return *seat_; }

	X11Output& createOutput(const nytl::vec2i& position = {0, 0}, 
			const nytl::vec2ui& size = {1400, 700});

	virtual std::unique_ptr<SurfaceContext> createSurfaceContext() const override;
	virtual WaylandEglContext* eglContext() const override { return eglContext_.get(); }
};


///Output for the x11 backend.
class X11Output : public Output
{
protected:
    xcb_window_t xWindow_;
	X11Backend* backend_;
	void* eglSurface_;

public:
    X11Output(X11Backend& backend, unsigned int id, const nytl::vec2i& pos, 
			const nytl::vec2ui& size);
    virtual ~X11Output();

    xcb_window_t xWindow() const { return xWindow_; }
	X11Backend& backend() const { return *backend_; }
	void* eglSurface() const { return eglSurface_; }

	virtual void sendInformation(const OutputRes& res) const override;
	virtual void redraw() override;
};

}
