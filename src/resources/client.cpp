#include <resources/client.hpp>
#include <compositor/compositor.hpp>
#include <seat/seat.hpp>
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
    if(!seat_)
    {
        for(auto* r : resources_)
        {
            if(r->getType() == resourceType::seat)
                seat_ = (seatRes*) r;
        }
    }

    return seat_;
}

pointerRes* client::getPointerRes() const
{
    return (getSeatRes() != nullptr) ? seat_->getPointerRes() : nullptr;
}

keyboardRes* client::getKeyboardRes() const
{
    return (seat_ != nullptr) ? seat_->getKeyboardRes() : nullptr;
}
