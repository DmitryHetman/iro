#include <iro/compositor/client.hpp>
#include <iro/compositor/compositor.hpp>
#include <iro/seat/seat.hpp>

#include <nytl/log.hpp>
#include <nytl/make_unique.hpp>

#include <wayland-server-core.h>

#include <signal.h>

#include <algorithm>
#include <iostream>

namespace iro
{

struct Client::listenerPOD
{
	wl_listener listener;
	Client* client;
};

//callbacks
void clientDestroyListener(struct wl_listener* listener, void*)
{
	Client::listenerPOD* pod;
	pod = wl_container_of(listener, pod, listener);	

	if(!pod || !pod->client)
	{
		nytl::sendWarning("Received a clientDestroy notify for unknown wl_client");
		return;	
	}	

	pod->client->compositor().unregisterClient(*pod->client);
}

//static client
Client* Client::find(wl_client& client)
{
	wl_listener* listener = wl_client_get_destroy_listener(&client, &clientDestroyListener);
	if(listener)
	{
		Client::listenerPOD* pod;
		pod = wl_container_of(listener, pod, listener);	

		if(pod) return pod->client;
		else return nullptr;
	}
	else
	{
		return nullptr;
	}
}

Client* Client::findWarn(wl_client& client)
{
	auto c = find(client);
	if(!c) nytl::sendWarning("cant find client object for wl_client ", &client);

	return c;
}

//client implementation
Client::Client(Compositor& comp, wl_client& wlc) : wlClient_(&wlc), compositor_(&comp)
{
	nytl::sendLog("creating client ", this, " for wl_client ", &wlc);

	listener_ = nytl::make_unique<listenerPOD>();
	listener_->listener.notify = clientDestroyListener; 
	listener_->client = this;

	wl_client_add_destroy_listener(wlClient_, &listener_->listener);
}

Client::~Client()
{
	nytl::sendLog("destructing client ", this, " with wl_client ", &wlClient(), ", there are ", 
			resources_.size(), " resources left");

	resources_.clear();
	destructionCallback_(*this);

	if(listener_) wl_list_remove(&listener_->listener.link);
}

void Client::destroy()
{
	nytl::sendLog("client::destroy: client ", this, " with wl_client ", &wlClient());
	wl_client_destroy(&wlClient());
}

void Client::addResource(std::unique_ptr<Resource>&& res)
{
    resources_.push_back(std::move(res));
}

bool Client::removeResource(const Resource& res)
{
	for(auto it = resources_.cbegin(); it != resources_.end(); ++it)
    {
		if(it->get() == &res)
		{
			resources_.erase(it);
			return 1;
		}
    }

	nytl::sendWarning("client::removeResource: resource ", &res, " was not found");
    return 0;
}

Resource* Client::findResource(const wl_resource& res) const
{
	for(auto& r: resources_)
		if(&r->wlResource() == &res) return r.get();

	return nullptr;
}

SeatRes* Client::seatResource() const
{
    for(auto& res : resources_)
    {
        if(res->type() == resourceType::seat)
            return (SeatRes*)res.get();
    }

    return nullptr;
}

PointerRes* Client::pointerResource() const
{
    for(auto& res : resources_)
    {
        if(res->type() == resourceType::pointer)
            return (PointerRes*)res.get();
    }

    return nullptr;
}

KeyboardRes* Client::keyboardResource() const
{
    for(auto& res : resources_)
    {
        if(res->type() == resourceType::keyboard)
            return (KeyboardRes*)res.get();
    }

    return nullptr;
}

int Client::pid() const
{
	pid_t pid;
	wl_client_get_credentials(&wlClient(), &pid, nullptr, nullptr);
	return pid;
}

int Client::uid() const
{
	uid_t uid;
	wl_client_get_credentials(&wlClient(), nullptr, &uid, nullptr);
	return uid;
}

int Client::gid() const
{
	gid_t gid;
	wl_client_get_credentials(&wlClient(), nullptr, nullptr, &gid);
	return gid;
}

bool Client::killProcess(unsigned int signal) const
{
	return kill(pid(), signal);
}

}
