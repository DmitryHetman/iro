#include <iro/compositor/client.hpp>
#include <iro/compositor/compositor.hpp>
#include <iro/seat/seat.hpp>
#include <iro/util/log.hpp>

#include <wayland-server-core.h>

#include <algorithm>
#include <iostream>

/////////////////////
void destroyClient(struct wl_listener* listener, void* data)
{
    wl_client* wlc = (wl_client*) data;

    client& c = iroCompositor()->getClient(*wlc);
    delete &c;
}

struct wl_listener onClientDestroy =
{
    wl_list(),
    destroyClient
};

//////////////////////////////////////////////////////////
client::client(wl_client& wlc) : wlClient_(wlc)
{
    iroLog("creating client ", this, " for wl_client ", &wlc);
    wl_client_add_destroy_listener(&wlc, &onClientDestroy);
}

client::~client()
{
    iroLog("destructing client ", this, " with wl_client ", &getWlClient(), ", there are ", resources_.size(), " resources left");
    iroCompositor()->unregisterClient(*this);
}

void client::addResource(resource& res)
{
    resources_.push_back(&res);
}

bool client::removeResource(resource& res)
{
    auto it = std::find(resources_.begin(), resources_.end(), &res);
    if(it != resources_.end())
    {
        resources_.erase(it);
        return 1;
    }

    iroWarning("client::removeResource: resource ", &res, " was not found");
    return 0;
}

seatRes* client::getSeatRes() const
{
    for(resource* res : resources_)
    {
        if(res->getType() == resourceType::seat)
            return (seatRes*)res;
    }

    return nullptr;
}

pointerRes* client::getPointerRes() const
{
    for(resource* res : resources_)
    {
        if(res->getType() == resourceType::pointer)
            return (pointerRes*)res;
    }

    return nullptr;
}

keyboardRes* client::getKeyboardRes() const
{
    for(resource* res : resources_)
    {
        if(res->getType() == resourceType::keyboard)
            return (keyboardRes*)res;
    }

    return nullptr;
}
