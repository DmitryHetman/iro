#include <resources/resource.hpp>
#include <resources/client.hpp>
#include <compositor/compositor.hpp>

#include <wayland-server-core.h>

#include <iostream>
#include <stdexcept>

void destroyResource(wl_resource* res)
{
    resource* mres = (resource*) wl_resource_get_user_data(res);
    if(mres) delete mres;
}

/////////////////////////////////////////////////////////
resource::resource(wl_resource* res) : wlResource_(res)
{
    getCompositor()->getClient(getWlClient())->addResource(this);
}

resource::resource(wl_client* client, unsigned int id, const struct wl_interface* interface, const void* implementation, unsigned int version, void* data, wl_resource_destroy_func_t destroyFunc)
{
    create(client, id, interface, implementation, version, data, destroyFunc);
}

resource::~resource()
{
}

void resource::destroy()
{
    wl_resource_destroy(wlResource_);
}

void resource::create(wl_client* client, unsigned int id, const struct wl_interface* interface, const void* implementation, unsigned int version, void* data, wl_resource_destroy_func_t destroyFunc)
{
    if(data == nullptr || 1)
        data = this;

    if(!destroyFunc)
        destroyFunc = destroyResource;

    wlResource_ = wl_resource_create(client, interface, version, id);
    if(!wlResource_)
    {
        std::string error = "failed to create resource for " + std::string(interface->name) + ", id " + std::to_string(id) + ", version " + std::to_string(version);
        throw std::runtime_error(error);
        return;
    }

    wl_resource_set_implementation(wlResource_, implementation, data, destroyFunc);
}

wl_client* resource::getWlClient() const
{
    return (wlResource_) ? wl_resource_get_client(wlResource_) : nullptr;
}

client* resource::getClient() const
{
    return getCompositor()->getClient(getWlClient());
}
