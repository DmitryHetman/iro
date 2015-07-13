#include <resources/client.hpp>
#include <compositor/compositor.hpp>

#include <wayland-server-core.h>

#include <algorithm>

void destroyClient(struct wl_listener* listener, void* data)
{
    client* c = getCompositor()->getClient((wl_client*) data);
    delete c;
}

struct wl_listener onClientDestroy =
{
    wl_list(),
    destroyClient
};

//////////////////////////////////////////////////////////
client::client(wl_client* wlc) : wlClient_(wlc)
{
    wl_client_add_destroy_listener(wlc, &onClientDestroy);
}

client::~client()
{
    getCompositor()->unregisterClient(this);

    if(resources_.size() > 0)
    {
        //warning?
    }
}

void client::addResource(resource* res)
{
    if(res->getType() == resourceType::seat)
    {
        seat_ = (seatRes*) res;
    }

    resources_.push_back(res);
}

void client::removeResource(resource* res)
{
    auto it = std::find(resources_.begin(), resources_.end(), res);
    if(it != resources_.end())
    {
        resources_.erase(it);
    }
    else
    {
        //warning?
    }
}
