#include <resources/resource.hpp>
#include <stdexcept>

resource::resource(wl_client* client, unsigned int id, const struct wl_interface* interface, const void* implementation, unsigned int version, void* data, wl_resource_destroy_func_t destroyFunc)
{
    create(client, id, interface, implementation, version, data, destroyFunc);
}

resource::~resource()
{
    //wl_resource_destroy(wlResource_);
}

void resource::create(wl_client* client, unsigned int id, const struct wl_interface* interface, const void* implementation, unsigned int version, void* data, wl_resource_destroy_func_t destroyFunc)
{
    if(data == nullptr)
        data = this;

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
