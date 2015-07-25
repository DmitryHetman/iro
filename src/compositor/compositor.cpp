#include <compositor/compositor.hpp>
#include <compositor/subcompositor.hpp>
#include <shell/shell.hpp>

#include <resources/surface.hpp>
#include <resources/region.hpp>
#include <resources/client.hpp>

#include <seat/seat.hpp>
#include <log.hpp>

#include <backend/backend.hpp>
#include <backend/x11/x11Backend.hpp>
#include <backend/kms/kmsBackend.hpp>

#include <wayland-server-protocol.h>

#include <stdexcept>
#include <iostream>

compositor* compositor::object = nullptr;
compositor* iroCompositor()
{
    return compositor::getObject();
}

subcompositor* getSubcompositor()
{
    if(!iroCompositor()) return nullptr;
    return iroCompositor()->getSubcompositor();
}

seat* iroSeat()
{
    if(!iroCompositor()) return nullptr;
    return iroCompositor()->getSeat();
}

pointer* iroPointer()
{
    if(!iroSeat()) return nullptr;
    return iroSeat()->getPointer();
}

keyboard* iroKeyboard()
{
    if(!iroSeat()) return nullptr;
    return iroSeat()->getKeyboard();
}

shell* iroShell()
{
    if(!iroCompositor()) return nullptr;
    return iroCompositor()->getShell();
}

wl_display* iroWlDisplay()
{
    if(!iroCompositor()) return nullptr;
    return iroCompositor()->getWlDisplay();
}

wl_event_loop* iroWlEventLoop()
{
    if(!iroCompositor()) return nullptr;
    return iroCompositor()->iroWlEventLoop();
}

unsigned int iroNextSerial()
{
    return (iroWlDisplay()) ? wl_display_next_serial(iroWlDisplay()) : 0;
}

event* iroGetEvent(unsigned int serial)
{
    return (iroCompositor()) ? iroCompositor()->getEvent(serial) : nullptr;
}

void iroRegisterEvent(event& ev)
{
    if(iroCompositor())iroCompositor()->registerEvent(ev);
    else iroWarning("iroRegisterEvent: no initialized compositor");
}

////////////////////////////////////////////////////////////////////////////////////
void compositorCreateSurface(wl_client* client, wl_resource* resource, unsigned int id)
{
    new surfaceRes(*client, id);
}
void compositorCreateRegion(wl_client* client, wl_resource* resource, unsigned int id)
{
    new regionRes(*client, id);
}

const struct wl_compositor_interface compositorImplementation =
{
    &compositorCreateSurface,
    &compositorCreateRegion
};

void bindCompositor(wl_client* client, void* data, unsigned int version, unsigned int id)
{
    new compositorRes(*client, id, version);
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
}

int compositor::run()
{
    wl_display_run(wlDisplay_);
    return 1;
}

wl_event_loop* compositor::iroWlEventLoop() const
{
    return wl_display_get_event_loop(wlDisplay_);
}

client& compositor::getClient(wl_client& wlc)
{
    if(clients_[&wlc] == nullptr)
    {
        clients_[&wlc] = new client(wlc);
    }

    return *clients_[&wlc];
}

void compositor::unregisterClient(client& c)
{
    auto it = clients_.find(&c.getWlClient());
    if(it != clients_.end())
    {
        clients_.erase(it->first);
    }
}

event* compositor::getEvent(unsigned int serial) const
{
    auto it = sentEvents_.find(serial);
    if(it != sentEvents_.end())
    {
        return it->second;
    }
    return nullptr;
}

void compositor::registerEvent(event& ev)
{
    sentEvents_[wl_display_get_serial(wlDisplay_)] = &ev;
}

////////////////////////////////////
compositorRes::compositorRes(wl_client& client, unsigned int id, unsigned int version) : resource(client, id, &wl_compositor_interface, &compositorImplementation, version)
{
}
