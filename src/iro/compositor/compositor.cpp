#include <iro/compositor/compositor.hpp>
#include <iro/compositor/subcompositor.hpp>
#include <iro/compositor/shell.hpp>

#include <iro/compositor/surface.hpp>
#include <iro/compositor/region.hpp>
#include <iro/compositor/client.hpp>

#include <iro/seat/seat.hpp>
#include <iro/util/log.hpp>

#include <iro/backend/backend.hpp>
#include <iro/backend/x11Backend.hpp>
#include <iro/backend/kmsBackend.hpp>

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

wl_display* iroWlDisplay()
{
    if(!iroCompositor()) return nullptr;
    return iroCompositor()->getWlDisplay();
}

wl_event_loop* iroWlEventLoop()
{
    if(!iroCompositor()) return nullptr;
    return iroCompositor()->getWlEventLoop();
}

unsigned int iroNextSerial(event* ev)
{
    if(iroCompositor() && ev) return iroCompositor()->registerEvent(*ev);
    else if(iroWlDisplay()) return wl_display_next_serial(iroWlDisplay());
    else return 0;
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
    seat_ = new seat();
}

compositor::~compositor()
{
    if(subcompositor_) delete subcompositor_;
    if(seat_) delete seat_;

    //if(wlDisplay_) wl_display_destroy(wlDisplay_);
}

void compositor::run()
{
    wl_display_run(wlDisplay_);
}

wl_event_loop* compositor::getWlEventLoop() const
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

bool compositor::registeredClient(wl_client& wlc)
{
    auto it = clients_.find(&wlc);

    if(it == clients_.end()) return 0;
    else return 1;
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

unsigned int compositor::registerEvent(event& ev)
{
    if(sentEvents_.size() > 5000)
    {
        //clear it
    }

    unsigned int ret = wl_display_next_serial(wlDisplay_);
    sentEvents_[ret] = &ev;
    return ret;
}

////////////////////////////////////
compositorRes::compositorRes(wl_client& client, unsigned int id, unsigned int version) : resource(client, id, &wl_compositor_interface, &compositorImplementation, version)
{
}
