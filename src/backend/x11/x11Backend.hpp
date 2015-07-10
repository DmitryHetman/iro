#pragma once

#include <iro.hpp>
#include <backend/backend.hpp>
#include <backend/output.hpp>
#include <backend/egl.hpp>

#include <wayland-server-core.h>

#include <xcb/xcb.h>
#include <X11/Xlib.h>

///////////////////////////////////////
x11Backend* getXBackend();

class x11Backend : public backend
{
protected:
    friend int x11EventLoop(int fd, unsigned int mask, void* data);

    Display* xDisplay_ = nullptr;
    xcb_connection_t* xConnection_ = nullptr;
    xcb_screen_t* xScreen_ = nullptr;

    eglContext* eglContext_ = nullptr;

    int eventLoop(int fd, unsigned int mask);

    int outputIDForWindow(xcb_window_t win) const;

public:
    x11Backend();
    ~x11Backend();

    Display* getXDisplay() const { return xDisplay_; }
    xcb_connection_t* getXConnection() const { return xConnection_; }
    xcb_screen_t* getXScreen() const { return xScreen_; }

    eglContext* getEglContext() const { return eglContext_; }
    backendType getType() const { return backendType::x11; }

public:
    static bool available();
};

//////////////////////////////////////
class x11Output : public output
{
protected:
    xcb_window_t xWindow_;
    EGLSurface eglWindow_;

public:
    x11Output(const x11Backend& backend, unsigned int id);
    ~x11Output();

    void makeEglCurrent();
    void swapBuffers();

    vec2ui getSize() const;

    xcb_window_t getXWindow() const { return xWindow_; }
};

