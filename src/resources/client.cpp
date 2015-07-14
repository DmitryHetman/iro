#include <resources/client.hpp>
#include <compositor/compositor.hpp>

#include <wayland-server-core.h>

#include <algorithm>
#include <iostream>

void addClientResource(wl_client* c, resource* res)
{
    getCompositor()->getClient(c)->addResource(res);
}

/////////////////////
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
    std::cout << "create client " << wlc << std::endl;
    wl_client_add_destroy_listener(wlc, &onClientDestroy);
    std::cout << "c2 " << std::endl;
}

client::~client()
{
    std::cout << "client destroyed " << wlClient_ << std::endl;

    getCompositor()->unregisterClient(this);

    if(resources_.size() > 0)
    {
        //warning?
    }
}

void client::addResource(resource* res)
{
    std::cout << "addRes " << (int)res->getType() << std::endl;

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
