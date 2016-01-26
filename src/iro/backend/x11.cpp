#include <iro/backend/x11.hpp>

#include <iro/compositor/compositor.hpp>
#include <iro/compositor/surface.hpp>
#include <iro/seat/seat.hpp>
#include <iro/seat/keyboard.hpp>
#include <iro/seat/pointer.hpp>
#include <iro/backend/egl.hpp>
#include <iro/backend/surfaceContext.hpp>

#include <ny/draw/gl/drawContext.hpp>

#include <ny/base/log.hpp>
#include <nytl/make_unique.hpp>

#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

#include <X11/Xlib-xcb.h>
#include <xcb/xcb_icccm.h>

#define explicit explicit_
#include <xcb/xkb.h>
#undef explicit

#include <xkbcommon/xkbcommon.h>

#include <linux/input.h>

#include <stdexcept>

namespace iro
{

namespace atoms
{
    xcb_atom_t protocols = 0;
    xcb_atom_t deleteWindow = 0;
}

//Backend
bool X11Backend::available()
{
    Display* test = XOpenDisplay(nullptr);
    if(!test)
    {
        return 0;
    }

    XCloseDisplay(test);
    return 1;
}

int X11Backend::eventCallback(int, unsigned int, void* data)
{
	X11Backend* b = static_cast<X11Backend*>(data);
	if(!b)
	{
		ny::sendWarning("X11Backend::EventCallback: invalid data");
		return 0;
	}

	return b->eventLoop();
}

X11Backend::X11Backend(Compositor& comp, Seat& seat) 
	: Backend(), compositor_(&comp), seat_(&seat)
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

    //atoms
    struct atomProp
    {
        xcb_atom_t& ret;
        std::string str; 
    };

   	atomProp vec[] = {
        {atoms::protocols, "WM_PROTOCOLS"},
        {atoms::deleteWindow, "WM_DELETE_WINDOW"}
    };

    xcb_intern_atom_reply_t* reply;
    for(auto& p : vec)
    {
        reply = xcb_intern_atom_reply(xConnection_, 
					xcb_intern_atom(xConnection_, 0, p.str.size(), p.str.c_str()), nullptr);
        p.ret = (reply ? reply->atom : 0);
    }

	//xkb
	xkbSetup();

    //event source
    inputEventSource_ =  wl_event_loop_add_fd(&comp.wlEventLoop(), 
			xcb_get_file_descriptor(xConnection_), WL_EVENT_READABLE, eventCallback, this);
    if(!inputEventSource_)
    {
        throw std::runtime_error("could not create wayland event source");
        return;
    }

	//what does this? really needed?
    wl_event_source_check(inputEventSource_);

	//eglContext
	eglContext_ = nytl::make_unique<WaylandEglContext>(xDisplay_);
	if(!eglContext_)
	{
        throw std::runtime_error("x11Backend::x11Backend: failed to create EglContext");
        return;
	}

	eglContext_->bindWlDisplay(compositor_->wlDisplay());


	xcb_flush(xConnection_);
}

X11Backend::~X11Backend()
{
	for(auto& outp : outputs_)
		destroyOutput(*outp);

	if(eglContext_)
	{
		eglContext_->unbindWlDisplay(compositor_->wlDisplay());
		eglContext_.reset();
	}

    if(inputEventSource_)wl_event_source_remove(inputEventSource_);
	if(xDisplay_) XCloseDisplay(xDisplay_);
}

void X11Backend::xkbSetup()
{
	const xcb_query_extension_reply_t *ext;
	if(!(ext = xcb_get_extension_data(xConnection_, &xcb_xkb_id)))
	{
		ny::sendWarning("xkb setup fail");
		return;
	}

	xkbEventBase_ = ext->first_event;
	xcb_void_cookie_t select = xcb_xkb_select_events_checked(xConnection_,
		XCB_XKB_ID_USE_CORE_KBD, XCB_XKB_EVENT_TYPE_STATE_NOTIFY, 0, 
		XCB_XKB_EVENT_TYPE_STATE_NOTIFY, 0, 0, nullptr);

	xcb_generic_error_t *error;
	if((error = xcb_request_check(xConnection_, select))) 
	{
		ny::sendWarning("xkb setup fail");
		free(error);
		return;
	}

	xcb_xkb_use_extension_reply_t *use_ext_reply;
	xcb_xkb_use_extension_cookie_t use_ext = xcb_xkb_use_extension(xConnection_, 
		XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);

	if(!(use_ext_reply = xcb_xkb_use_extension_reply(xConnection_, use_ext, nullptr)))
   	{
   	 	ny::sendWarning("xkb setup fail");
   	 	free(error);
   	}

   	const bool supported = use_ext_reply->supported;
   	free(use_ext_reply);

   	if(!supported)
   	{
   	 	ny::sendWarning("xkb setup fail");
   	 	free(error);
   	}

   	xcb_xkb_per_client_flags_cookie_t pcf = xcb_xkb_per_client_flags(xConnection_, 
   	 	XCB_XKB_ID_USE_CORE_KBD, XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT, 
   	 	XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT, 0, 0, 0);

   	xcb_xkb_per_client_flags_reply_t *pcf_reply;
   	if(!(pcf_reply = xcb_xkb_per_client_flags_reply(xConnection_, pcf, NULL)))
   	{
   	 	ny::sendWarning("xkb setup fail");
   	 	free(error);
   	}

   	const bool hasRepeat = (pcf_reply->value & XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT);
   	free(pcf_reply);

   	if(!hasRepeat)
   	{
   	 	ny::sendWarning("xkb setup fail");
   	 	free(error);
   	}
}

std::unique_ptr<SurfaceContext> X11Backend::createSurfaceContext() const
{
	return nytl::make_unique<DefaultSurfaceContext>(*eglContext());
}

X11Output& X11Backend::createOutput(const nytl::vec2i& position, const nytl::vec2ui& size)
{
	auto outp = nytl::make_unique<X11Output>(*this, outputs_.size() + 1, position, size);
	auto& ret = *outp;

	addOutput(std::move(outp));

	xcb_flush(xConnection_);
	return ret;
}

int X11Backend::eventLoop()
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
                auto* outp = outputForWindow(ev->window);
                if(!outp)
                {
					ny::sendWarning("xcb_expose: invalid xcb window");
                    break;
                }

                outp->scheduleRepaint();
                break;
			}
            case XCB_CLIENT_MESSAGE:
            {
                xcb_client_message_event_t* ev = (xcb_client_message_event_t*) event;
                if(ev->data.data32[0] == atoms::deleteWindow)
                {
                    auto* outp = outputForWindow(ev->window);
                    if(!outp)
                    {
						ny::sendWarning("xcb_client_message: invalid xcb window");
                        break;
                    }
					
					destroyOutput(*outp);

                    if(outputs_.empty())
                    {
                        compositor().exit();
                        return count + 1;
                    }
                }
                break;
            }
            case XCB_BUTTON_PRESS:
            {
				if(!seat().pointer()) break;

                xcb_button_press_event_t* ev = (xcb_button_press_event_t*) event;
                unsigned int code = (ev->detail == 2 ? BTN_MIDDLE : 
						(ev->detail == 3 ? BTN_RIGHT : ev->detail + BTN_LEFT - 1));
                seat().pointer()->sendButton(code, 1);
                break;
            }
            case XCB_BUTTON_RELEASE:
            {
				if(!seat().pointer()) break;

                xcb_button_release_event_t* ev = (xcb_button_release_event_t*) event;
                unsigned int code = (ev->detail == 2 ? BTN_MIDDLE : 
						(ev->detail == 3 ? BTN_RIGHT : ev->detail + BTN_LEFT - 1));
                seat().pointer()->sendButton(code, 0);
                break;
            }
            case XCB_MOTION_NOTIFY:
            {
				if(!seat().pointer()) break;

                xcb_motion_notify_event_t* ev = (xcb_motion_notify_event_t*) event;
                seat().pointer()->sendMove(nytl::vec2i(ev->event_x, ev->event_y));
                break;
            }
            case XCB_KEY_PRESS:
            {
				if(!seat().keyboard()) break;

                xcb_key_press_event_t* ev = (xcb_key_press_event_t*) event;
                seat().keyboard()->sendKey(ev->detail - 8, 1);
                break;
            }
            case XCB_KEY_RELEASE:
            {
				if(!seat().keyboard()) break;

                xcb_key_press_event_t* ev = (xcb_key_press_event_t*) event;
                seat().keyboard()->sendKey(ev->detail - 8, 0);
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

		if(event->response_type == xkbEventBase_)
		{
			xcb_xkb_state_notify_event_t *ev = (xcb_xkb_state_notify_event_t*)event;
			if(ev->xkbType == XCB_XKB_STATE_NOTIFY)
			{
				if(seat().keyboard())
				{
					auto& kb = *seat().keyboard();
					xkb_state_update_mask(kb.xkbState(), kb.modMask(ev->baseMods), 
						kb.modMask(ev->latchedMods), kb.modMask(ev->lockedMods), 0, 0, ev->group);
						
					seat().keyboard()->updateModifiers();
				}	
			}
		}

        free(event);
		++count;
	}

	return count;
}

X11Output* X11Backend::outputForWindow(const xcb_window_t& win)
{
	for(auto& outp : outputs_)
	{
		auto* xout = static_cast<X11Output*>(outp.get());
		if(xout->xWindow() == win)
			return xout;
	}

	return nullptr;
}


//Output
X11Output::X11Output(X11Backend& backend, unsigned int id, const nytl::vec2i& pos, 
		const nytl::vec2ui& size) : Output(backend.compositor(), id, pos, size), backend_(&backend)
{
    xcb_connection_t* connection = backend.xConnection();
    xcb_screen_t* screen = backend.xScreen();

    unsigned int mask = XCB_CW_EVENT_MASK;
    unsigned int values = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS | 
		XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_KEY_PRESS |
        XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW | 
		XCB_EVENT_MASK_FOCUS_CHANGE;

    xWindow_ = xcb_generate_id(connection);
    xcb_create_window(connection, XCB_COPY_FROM_PARENT, xWindow_, screen->root, 0, 0, size_.x, 
			size_.y ,10, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, mask, &values);

    if(!xWindow_)
    {
        throw std::runtime_error("could not create xcb window");
        return;
    }

    xcb_change_property(connection, XCB_PROP_MODE_REPLACE, xWindow_, atoms::protocols, 
			XCB_ATOM_ATOM, 32, 1, &atoms::deleteWindow);

    //cant be resized
    xcb_size_hints_t sizeHints;

    sizeHints.max_width = size_.x;
    sizeHints.max_height = size_.y;

    sizeHints.min_width = size_.x;
    sizeHints.min_height = size_.y;

    sizeHints.flags = XCB_ICCCM_SIZE_HINT_P_MIN_SIZE | XCB_ICCCM_SIZE_HINT_P_MAX_SIZE;

    xcb_icccm_set_wm_size_hints(connection, xWindow_, XCB_ATOM_WM_NORMAL_HINTS, &sizeHints);

    //no mouse cursor
    xcb_pixmap_t cursorPixmap = xcb_generate_id(connection);
    xcb_create_pixmap(connection, 1, cursorPixmap, xWindow_, 1, 1);

    xcb_cursor_t hiddenCursor = xcb_generate_id(connection);

    xcb_create_cursor(connection, hiddenCursor, cursorPixmap, cursorPixmap, 
			0, 0, 0, 0, 0, 0, 0, 0);
    xcb_free_pixmap(connection, cursorPixmap);

    xcb_change_window_attributes(connection, xWindow_, XCB_CW_CURSOR, &hiddenCursor);
    xcb_map_window(connection, xWindow_);

	//EGL
	if(!backend.eglContext())
	{
		throw std::runtime_error("x11Output::x11Output: backend ha no valid eglcontext");
		return;
	}

	eglSurface_ = backend.eglContext()->createSurface(xWindow_);
	if(!eglSurface_)
	{
		throw std::runtime_error("x11Output::x11Output: failed to create egl surface");
		return;
	}

	drawContext_ = nytl::make_unique<ny::GlDrawContext>();
	if(!drawContext_)
	{
		throw std::runtime_error("x11Output::x11Output: failed to create ny::GlesDC");
		return;
	}
}

X11Output::~X11Output()
{
	if(eglSurface_)backend().eglContext()->destroySurface(eglSurface_);
    xcb_destroy_window(backend().xConnection(), xWindow_);
}

void X11Output::sendInformation(const OutputRes& res) const
{
	wl_output_send_geometry(&res.wlResource(), position_.x, position_.y, size_.x, size_.y, 
			0, "", "", WL_OUTPUT_TRANSFORM_NORMAL);
	wl_output_send_done(&res.wlResource());
}


void X11Output::redraw()
{
	ny::sendLog("X11Output:redraw: id ", id());
	if(!backend().eglContext()->makeCurrentForSurface(eglSurface_))
	{
		ny::sendWarning("X11Output::redraw: failed to make eglContext current");
		return;
	}

	Output::redraw();

	if(!backend().eglContext()->apply())
	{
		ny::sendWarning("X11Output::redraw: failed to swap egl buffers");
	}
	
	backend().eglContext()->makeNotCurrent();
}

}
