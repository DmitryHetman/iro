#include <iro/compositor/resource.hpp>
#include <iro/compositor/client.hpp>
#include <iro/compositor/compositor.hpp>

#include <ny/base/log.hpp>
#include <nytl/make_unique.hpp>

#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

#include <iostream>
#include <stdexcept>

namespace iro
{

struct Resource::listenerPOD
{
	wl_listener listener;
	Resource* resource;
};

//callbacks
void resourceDestroyListener(wl_listener* listener, void*)
{
	Resource::listenerPOD* pod;
	pod = wl_container_of(listener, pod, listener);

	if(!pod || !pod->resource)
	{
		ny::sendWarning("resourceDestroyListener with invalid data paremeter");
		return;
	}

	pod->resource->client().removeResource(*pod->resource);
}

//static resource utility
Resource* Resource::find(wl_resource& res)
{
	wl_listener* listener = wl_resource_get_destroy_listener(&res, &resourceDestroyListener);
	if(listener)
	{
		Resource::listenerPOD* pod;
		pod = wl_container_of(listener, pod, listener);	

		if(pod) return pod->resource;
		else return nullptr;
	}
	else
	{
		return nullptr;
	}
}

void Resource::invalidObjectDisconnect(wl_resource& res, const std::string& info)
{
	std::string inf = info.empty() ? "" : " additional info: " + info;

	auto* clnt = wl_resource_get_client(&res);
	wl_resource_post_error(&res, WL_DISPLAY_ERROR_INVALID_OBJECT, "object %p is invalid or was"
		   " used in an invalid context. Client will be disconnected.%s", &res, inf.c_str());

	ny::sendWarning("Resource::invalidObjectDisconnect: wl_client ", clnt, 
			" used invalid object ", &res, " and will be disconnected.", inf);

	//store event that client will be destroyed after event loop is fully dispatched - problems
	//otherwise...
	//wl_client_destroy(clnt);	
}

//resource implementation
Resource::Resource(wl_resource& res) : wlResource_(&res), listener_(nullptr)
{
	listener_ = nytl::make_unique<listenerPOD>();

	listener_->listener.notify = resourceDestroyListener;
	listener_->resource = this;

	wl_resource_add_destroy_listener(&res, &listener_->listener);
}

Resource::Resource(wl_client& client, unsigned int id, const struct wl_interface* interface, 
		const void* implementation, unsigned int version) : listener_(nullptr)
{
    create(client, id, interface, implementation, version);
}

Resource::~Resource()
{
	ny::sendLog("destructing resource ",this, " with id ", id()," of wl_client ", &wlClient());

    if(wl_resource_get_user_data(wlResource_) == this)
		wl_resource_set_user_data(wlResource_, nullptr);

	if(listener_) wl_list_remove(&listener_->listener.link);
}

void Resource::destroy()
{
    if(wlResource_) wl_resource_destroy(wlResource_);
}

void Resource::create(wl_client& client, unsigned int id, const wl_interface* interface, 
		const void* implementation, unsigned int version)
{
	ny::sendLog("new resource ",this, " with id ", id, " and type ", (interface) ? 
			interface->name : "<unknown>", ", version ", version, " for wl_client ", &client);

    wlResource_ = wl_resource_create(&client, interface, version, id);
    if(!wlResource_)
    {
        //todo: really throw here?? its a runtime function...
        std::string error = "failed to create resource for " + std::string(interface->name) + 
			", id " + std::to_string(id) + ", version " + std::to_string(version);

        throw std::runtime_error(error);
        return;
    }

    wl_resource_set_implementation(wlResource_, implementation, this, nullptr);

	listener_ = nytl::make_unique<listenerPOD>();
	listener_->listener.notify = resourceDestroyListener;
	listener_->resource = this;

	wl_resource_add_destroy_listener(wlResource_, &listener_->listener);
}

wl_client& Resource::wlClient() const
{
    return *(wl_resource_get_client(wlResource_));
}

Client& Resource::client() const
{
    return *Client::find(wlClient());
}

Compositor& Resource::compositor() const
{
	return client().compositor();
}

unsigned int Resource::id() const
{
    return wl_resource_get_id(wlResource_);
}

unsigned int Resource::version() const
{
	return wl_resource_get_version(wlResource_);
}

//equal operator
bool operator==(const Resource& r1, const Resource& r2)
{
    return (r1.id() == r2.id() && &r1.wlClient() == &r2.wlClient() && 
			r1.wlResource() == r2.wlResource());
}

}
