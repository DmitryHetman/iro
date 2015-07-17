#include <resources/client.hpp>
#include <compositor/compositor.hpp>

#include <log.hpp>

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
    wl_client_add_destroy_listener(&wlc, &onClientDestroy);
}

client::~client()
{
    iroCompositor()->unregisterClient(this);
}

void client::addResource(resource& res)
{
    resources_.push_back(&res);

    if(res.getType() == resourceType::seat)
    {
        seat_ = (seatRes*) &resources_.back();
    }
}

bool client::removeResource(resource& res)
{
    auto it = std::find(resources_.begin(), resources_.end(), &res);
    if(it != resources_.end())
    {
        resources_.erase(it);
        return 1;
    }

    iroWarning("resource that should be removed was not found");
    return 0;
}
