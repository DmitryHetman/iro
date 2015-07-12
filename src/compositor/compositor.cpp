#include <compositor/compositor.hpp>
#include <compositor/subcompositor.hpp>
#include <shell/shell.hpp>

#include <resources/surface.hpp>
#include <resources/region.hpp>

#include <seat/seat.hpp>

#include <backend/backend.hpp>
#include <backend/x11/x11Backend.hpp>
#include <backend/kms/kmsBackend.hpp>

#include <wayland-server-protocol.h>

#include <stdexcept>
#include <iostream>

compositor* compositor::object = nullptr;
compositor* getCompositor()
{
    return compositor::getObject();
}

subcompositor* getSubcompositor()
{
    if(!getCompositor()) return nullptr;
    return getCompositor()->getSubcompositor();
}

backend* getBackend()
{
    if(!getCompositor()) return nullptr;
    return getCompositor()->getBackend();
}

seat* getSeat()
{
    if(!getCompositor()) return nullptr;
    return getCompositor()->getSeat();
}

shell* getShell()
{
    if(!getCompositor()) return nullptr;
    return getCompositor()->getShell();
}

wl_display* getWlDisplay()
{
    if(!getCompositor()) return nullptr;
    return getCompositor()->getWlDisplay();
}

wl_event_loop* getWlEventLoop()
{
    if(!getCompositor()) return nullptr;
    return getCompositor()->getWlEventLoop();
}

////////////////////////////////////////////////////////////////////////////////////
void compositorCreateSurface(wl_client* client, wl_resource* resource, unsigned int id)
{
    new surfaceRes(client, id);
}
void compositorCreateRegion(wl_client* client, wl_resource* resource, unsigned int id)
{
    new regionRes(client, id);
}

const struct wl_compositor_interface compositorImplementation =
{
    &compositorCreateSurface,
    &compositorCreateRegion
};

void bindCompositor(wl_client* client, void* data, unsigned int version, unsigned int id)
{
    new compositorRes(client, id, version);
}

/////////////////////////////////////////////////////////////////////////////////77

compositor::compositor()
{
    if(!(wlDisplay_ = wl_display_create()))
    {
        throw std::runtime_error("could not create wayland display");
    }

    wl_display_add_socket_auto(wlDisplay_);

    if(wl_display_init_shm(wlDisplay_) != 0)
    {
        throw std::runtime_error("could not initialize wayland shm");
    }

    if(!wl_global_create(wlDisplay_, &wl_compositor_interface, 3, this, bindCompositor))
    {
        throw std::runtime_error("could not create wayland compositor");
    }

    object = this;


    if(x11Backend::available())
        backend_ = new x11Backend();
    else
        backend_ = new kmsBackend();

    subcompositor_ = new subcompositor();
    shell_ = new shell();
    seat_ = new seat();
}

compositor::~compositor()
{
    //if(wlDisplay_) wl_display_destroy(wlDisplay_);

    if(subcompositor_) delete subcompositor_;
    if(shell_) delete shell_;
    if(seat_) delete seat_;
    if(backend_) delete backend_;
}

int compositor::run()
{
    wl_display_run(wlDisplay_);
    return 1;
}

wl_event_loop* compositor::getWlEventLoop() const
{
    return wl_display_get_event_loop(wlDisplay_);
}

////////////////////////////////////
compositorRes::compositorRes(wl_client* client, unsigned int id, unsigned int version) : resource(client, id, &wl_compositor_interface, &compositorImplementation, version)
{
}
