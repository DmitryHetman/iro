#include <iro/compositor/compositor.hpp>

#include <iro/compositor/surface.hpp>
#include <iro/compositor/region.hpp>
#include <iro/compositor/subcompositor.hpp>
#include <iro/compositor/client.hpp>
#include <iro/backend/backend.hpp>
#include <iro/seat/event.hpp>

#include <nytl/make_unique.hpp>
#include <nytl/log.hpp>

#include <wayland-server-protocol.h>

#include <signal.h>
#include <stdexcept>
#include <iostream>


namespace iro
{

//compositor resource
class CompositorRes : public Resource
{
protected:
	Compositor* compositor_;

public:
	CompositorRes(Compositor& comp, wl_client& client, unsigned int id, unsigned int v);
	Compositor& compositor() const { return *compositor_; }
};

namespace
{

//wayland implementation
void compositorCreateSurface(wl_client* client, wl_resource*, unsigned int id)
{
	auto clnt = Client::findWarn(*client);
	if(clnt) clnt->addResource(nytl::make_unique<SurfaceRes>(*client, id));
}
void compositorCreateRegion(wl_client* client, wl_resource*, unsigned int id)
{
	auto clnt = Client::findWarn(*client);
    if(clnt) clnt->addResource(nytl::make_unique<RegionRes>(*client, id));
}

const struct wl_compositor_interface compositorImplementation =
{
    &compositorCreateSurface,
    &compositorCreateRegion
};

void bindCompositor(wl_client* client, void* data, unsigned int version, unsigned int id)
{
	Compositor* comp = static_cast<Compositor*>(data);
	if(!comp) return;

    auto& clnt = comp->client(*client);
	clnt.addResource(nytl::make_unique<CompositorRes>(*comp, *client, id, version));
}

int signalIntHandler(int, void* data)
{
	nytl::sendLog("Received signal SIGINT. Exiting the compositor");

	if(!data) return 1;
	static_cast<Compositor*>(data)->exit();

	return 1;
}

int compExit(void* data)
{
	nytl::sendLog("terminating compositor, 5 seconds passed");
	static_cast<Compositor*>(data)->exit();
	return 1;
}

}

//compositor resource implementation
CompositorRes::CompositorRes(Compositor& comp, wl_client& clnt, unsigned int id, unsigned int v) 
	: Resource(clnt, id, &wl_compositor_interface, &compositorImplementation, v), compositor_(&comp)
{
}

//compositor implementation
Compositor::Compositor()
{
	timer_.reset();
	
    if(!(wlDisplay_ = wl_display_create()))
    {
        throw std::runtime_error("could not create wayland display");
    }

    wl_display_add_socket_auto(wlDisplay_);

    if(wl_display_init_shm(wlDisplay_) != 0)
    {
        throw std::runtime_error("could not initialize wayland shm");
    }

	wlGlobal_ = wl_global_create(wlDisplay_, &wl_compositor_interface, 3, this, bindCompositor);
    if(!wlGlobal_)
    {
        throw std::runtime_error("could not create wayland compositor global");
    }


	wl_event_loop_add_signal(&wlEventLoop(), SIGINT, signalIntHandler, this);
	subcompositor_.reset(new Subcompositor(*this));
}

Compositor::~Compositor()
{
	//cant destruct global after display was destroyed
	if(wlGlobal_) 
	{
		wl_global_destroy(wlGlobal_);
		wlGlobal_ = nullptr;
	}

	//before display gets destroyed
	subcompositor_.reset();

    if(wlDisplay_) wl_display_destroy(wlDisplay_);
}

void Compositor::run()
{
	wl_display_run(wlDisplay_);
}

void Compositor::run(const nytl::timeDuration& dur)
{
	auto* exitSrc = wl_event_loop_add_timer(&wlEventLoop(), compExit, this);
	wl_event_source_timer_update(exitSrc, dur.asMilliseconds());

	run();
}

void Compositor::exit()
{
	wl_display_terminate(wlDisplay_);
}

wl_event_loop& Compositor::wlEventLoop() const
{
    return *wl_display_get_event_loop(wlDisplay_);
}

Client& Compositor::client(wl_client& wlc)
{
	auto cr = clientRegistered(wlc);
	if(cr) return *cr;

	auto c = nytl::make_unique<Client>(*this, wlc);
	auto& ret = *c;

	clients_.push_back(std::move(c));
	return ret;
}

void Compositor::unregisterClient(Client& c)
{
	for(auto it = clients_.cbegin(); it != clients_.cend(); ++it)
	{
		if(it->get() == &c)
		{
			clients_.erase(it);
			return;
		}
	}
}

Client* Compositor::clientRegistered(wl_client& wlc) const
{
	for(auto& c : clients_)
		if(&c->wlClient() == &wlc)
			return c.get();

	return nullptr;
}

Event& Compositor::event(std::unique_ptr<Event>&& ptr, bool serial)
{
	auto& ret = *ptr;
	if(serial)
		ptr->serial = wl_display_next_serial(wlDisplay_);

	eventList_.push_back(std::move(ptr));
	return ret;
}

Event* Compositor::event(unsigned int serial) const
{
	for(auto& ev : eventList_)
		if(ev->serial == serial) return ev.get();

	return nullptr;
}

unsigned int Compositor::time() const
{
	return timer_.getElapsedTime().asMilliseconds();
}

}
