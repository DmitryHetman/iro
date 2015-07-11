#include <resources/resource.hpp>
#include <stdexcept>

#include <wayland-server-core.h>

#include <iostream>

void destroyResource(wl_resource* res)
{
    resource* mres = (resource*) wl_resource_get_user_data(res);

    std::cout << wl_resource_get_id(res) << " " << mres << std::endl;
    if(mres) delete mres;

    std::cout << "finished" << std::endl;
}

/////////////////////////////////////////////////////////
resource::resource(wl_resource* res) : wlResource_(res)
{
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
    std::cout << id << " " << interface->name << " " << this << std::endl;

    if(data == nullptr || 1)
        data = this;

    if(!destroyFunc)
        destroyFunc = destroyResource;

    wlResource_ = wl_resource_create(client, interface, version, id);
    if(!wlResource_)
    {
        std::string error;
        error.append("failed to create resource for ");
        error.append(interface->name);
        throw std::runtime_error(error);
        return;
    }

    wl_resource_set_implementation(wlResource_, implementation, data, destroyFunc);
}

wl_client* resource::getWlClient() const
{
    return wl_resource_get_client(wlResource_);
}
