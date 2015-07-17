#include <backend/x11/x11Backend.hpp>
#include <server.hpp>
#include <compositor/compositor.hpp>
#include <seat/seat.hpp>
#include <seat/keyboard.hpp>
#include <seat/pointer.hpp>

#include <wayland-server-core.h>

#include <backend/egl.hpp>
#include <backend/renderer.hpp>

#define explicit dont_use_cxx_explicit
#include <xcb/xkb.h>
#undef explicit

#include <X11/Xlib-xcb.h>
#include <xcb/xcb_icccm.h>

#include <string.h>
#include <dlfcn.h>
#include <stdexcept>
#include <iostream>

xcb_atom_t atomProtocols = 0;
xcb_atom_t atomDeleteWindow = 0;

int x11EventLoop(int fd, unsigned int mask, void* data);

x11Backend* getXBackend()
{
    if(!iroBackend()) return nullptr;
    return (x11Backend*)iroBackend();
}

//////////////////////////////////////////
x11Backend::x11Backend()
{
    //setup x connection
    xDisplay_ = XOpenDisplay(nullptr);
    if(!xDisplay_)
    {
        throw std::runtime_error("cant connect to x11 server");
        return;
    }

    xConnection_ = XGetXCBConnection(xDisplay_);
    if(!xConnection_)
    {
        throw std::runtime_error("cant get xcb connection");
        return;
    }

    XSetEventQueueOwner(xDisplay_, XCBOwnsEventQueue);

    if(xcb_connection_has_error(xConnection_))
    {
        throw std::runtime_error("xcb connection error");
        return;
    }

    xScreen_ = xcb_setup_roots_iterator(xcb_get_setup(xConnection_)).data;

    //egl
    eglContext_ = new eglContext(xDisplay_);

    //atoms
    xcb_intern_atom_reply_t* reply;

    struct atomProp
    {
        xcb_atom_t& ret;
        const char* str;
    } vec[] = {
        {atomProtocols, "WM_PROTOCOLS"},
        {atomDeleteWindow, "WM_DELETE_WINDOW"}
    };

    for(auto& p : vec)
    {
        reply = xcb_intern_atom_reply(xConnection_, xcb_intern_atom(xConnection_, 0, strlen(p.str), p.str), nullptr);
        p.ret = (reply ? reply->atom : 0);
    }

    //event source
    wlEventSource_ =  wl_event_loop_add_fd(iroWlEventLoop(), xcb_get_file_descriptor(xConnection_), WL_EVENT_READABLE, x11EventLoop, this);
    if(!wlEventSource_)
    {
        throw std::runtime_error("could not create wayland event source");
        return;
    }

    wl_event_source_check(wlEventSource_);

    //outputs
    unsigned int numOutputs = 1;
    for(unsigned int i(0); i < numOutputs; i++)
    {
        x11Output* o = new x11Output(*this, i);
        outputs_.push_back(o);
    }

    xcb_flush(xConnection_);

    renderer_ = new renderer();
}

x11Backend::~x11Backend()
{
    wl_event_source_remove(wlEventSource_);

    if(eglContext_) delete eglContext_;
    if(xDisplay_) XCloseDisplay(xDisplay_);
}

int x11Backend::eventLoop(int fd, unsigned int mask)
{
    xcb_generic_event_t* event;
    int count = 0;

    while((event = xcb_poll_for_event(xConnection_)))
    {
        switch(event->response_type & ~0x80)
        {
            case XCB_EXPOSE:
            {
                xcb_expose_event_t* ev = (xcb_expose_event_t*) event;
                int id = outputIDForWindow(ev->window);
                if(id == -1)
                {
                    iroWarning("invalid xcb window");
                    break;
                }
                outputs_[id]->refresh();
                break;
            }
            case XCB_CLIENT_MESSAGE:
            {
                xcb_client_message_event_t* ev = (xcb_client_message_event_t*) event;
                if(ev->data.data32[0] == atomDeleteWindow)
                {
                    int id = outputIDForWindow(ev->window);
                    if(id == -1)
                    {
                        iroWarning("invalid xcb window");
                        break;
                    }

                    delete outputs_[id];
                    outputs_.erase(outputs_.begin() + id);

                    if(outputs_.empty())
                    {
                        iroServer()->exit();
                        return count + 1;
                    }
                }

                break;
            }
            case XCB_BUTTON_PRESS:
            {
                xcb_button_press_event_t* ev = (xcb_button_press_event_t*) event;
                iroSeat()->getPointer()->sendButtonPress(ev->detail);
                break;
            }
            case XCB_BUTTON_RELEASE:
            {
                xcb_button_release_event_t* ev = (xcb_button_release_event_t*) event;
                iroSeat()->getPointer()->sendButtonRelease(ev->detail);
                break;
            }
            case XCB_MOTION_NOTIFY:
            {
                xcb_motion_notify_event_t* ev = (xcb_motion_notify_event_t*) event;
                iroSeat()->getPointer()->sendMove(ev->event_x, ev->event_y);
                break;
            }
            case XCB_KEY_PRESS:
            {
                xcb_key_press_event_t* ev = (xcb_key_press_event_t*) event;
                iroSeat()->getKeyboard()->sendKeyPress(ev->detail);
                break;
            }
            case XCB_KEY_RELEASE:
            {
                xcb_key_press_event_t* ev = (xcb_key_press_event_t*) event;
                iroSeat()->getKeyboard()->sendKeyRelease(ev->detail);
                break;
            }
            case XCB_FOCUS_IN:
            {
                break;
            }
            case XCB_FOCUS_OUT:
            {
                break;
            }
            case XCB_ENTER_NOTIFY:
            {
                break;
            }
            case XCB_LEAVE_NOTIFY:
            {
                break;
            }
            default: break;
        }

        free(event);
        count++;
    }

    xcb_flush(xConnection_);
    return count;
}

int x11Backend::outputIDForWindow(xcb_window_t win) const
{
    for(unsigned int i(0); i < outputs_.size(); i++)
    {
        x11Output* out = (x11Output*) outputs_[i];
        if(win == out->getXWindow())
            return i;
    }

    return -1;
}

////////////////////////////////
bool x11Backend::available()
{
    Display* test = XOpenDisplay(nullptr);
    if(!test)
    {
        return 0;
    }
    XCloseDisplay(test);
    return 1;
}

int x11EventLoop(int fd, unsigned int mask, void* data)
{
    x11Backend* b = static_cast<x11Backend*>(data);
    return b->eventLoop(fd, mask);
}

/////////////////////////////
x11Output::x11Output(const x11Backend& backend, unsigned int id) : output(id)
{
    xcb_connection_t* connection = backend.getXConnection();
    xcb_screen_t* screen = backend.getXScreen();

    eglContext* ctx = backend.getEglContext();

    const EGLint winAttribs[] = {
        EGL_NONE
    };

    unsigned int mask = XCB_CW_EVENT_MASK;
    unsigned int values = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_KEY_PRESS |
                            XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW | XCB_EVENT_MASK_FOCUS_CHANGE;

    xWindow_ = xcb_generate_id(connection);

    xcb_create_window(connection, XCB_COPY_FROM_PARENT, xWindow_, screen->root, 0, 0, 800, 500,10, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, mask, &values);
    if(!xWindow_)
    {
        throw std::runtime_error("could not create xcb window");
        return;
    }

    xcb_change_property(connection, XCB_PROP_MODE_REPLACE, xWindow_, atomProtocols, XCB_ATOM_ATOM, 32, 1, &atomDeleteWindow);

    //cant be resized
    xcb_size_hints_t sizeHints;

    sizeHints.max_width = 800;
    sizeHints.max_height = 500;

    sizeHints.min_width = 800;
    sizeHints.min_height = 500;

    sizeHints.flags = XCB_ICCCM_SIZE_HINT_P_MIN_SIZE | XCB_ICCCM_SIZE_HINT_P_MAX_SIZE;

    xcb_icccm_set_wm_size_hints(connection, xWindow_, XCB_ATOM_WM_NORMAL_HINTS, &sizeHints);

    //no mouse cursor
    xcb_pixmap_t cursorPixmap = xcb_generate_id(connection);
    xcb_create_pixmap(connection, 1, cursorPixmap, xWindow_, 1, 1);

    xcb_cursor_t hiddenCursor = xcb_generate_id(connection);

    xcb_create_cursor(connection, hiddenCursor, cursorPixmap, cursorPixmap,0, 0, 0, 0, 0, 0, 0, 0);
    xcb_free_pixmap(connection, cursorPixmap);

    xcb_change_window_attributes(connection, xWindow_, XCB_CW_CURSOR, &hiddenCursor);

    //egl
    eglWindow_ = eglCreateWindowSurface(ctx->getDisplay(), ctx->getConfig(), xWindow_, winAttribs);
    if(!eglWindow_)
    {
        throw std::runtime_error("could not create egl surface");
        return;
    }

    eglMakeCurrent(ctx->getDisplay(), eglWindow_, eglWindow_, ctx->getContext());

    xcb_map_window(connection, xWindow_);
}

x11Output::~x11Output()
{
    xcb_destroy_window(getXBackend()->getXConnection(), xWindow_);
}

void x11Output::makeEglCurrent()
{
    eglMakeCurrent(iroEglContext()->getDisplay(), eglWindow_, eglWindow_, iroEglContext()->getContext());
}

void x11Output::swapBuffers()
{
    eglSwapBuffers(iroEglContext()->getDisplay(), eglWindow_);
}

vec2ui x11Output::getSize() const
{
    vec2ui ret;
    ret.x = 800;
    ret.y = 500;
    return ret;
}

