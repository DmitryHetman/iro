#include <iro/compositor/resource.hpp>
#include <iro/compositor/client.hpp>
#include <iro/compositor/compositor.hpp>

#include <iro/util/log.hpp>

#include <wayland-server-core.h>

#include <iostream>
#include <stdexcept>

void destroyResource(wl_resource* res)
{
    resource* mres = (resource*) wl_resource_get_user_data(res);
    if(mres) delete mres;
}

/////////////////////////////////////////////////////////
resource::resource(wl_resource& res) : wlResource_(&res)
{
    getClient().addResource(*this);
}

resource::resource(wl_client& client, unsigned int id, const struct wl_interface* interface, const void* implementation, unsigned int version)
{
    create(client, id, interface, implementation, version);
    getClient().addResource(*this);
}

resource::~resource()
{
    iroLog("destructing resource ",this, " with id ", getID()," of wl_client ", &getWlClient());

    getClient().removeResource(*this);
    destructionCallback_();
}

void resource::destroy()
{
    wl_resource_destroy(wlResource_);
}

void resource::create(wl_client& client, unsigned int id, const struct wl_interface* interface, const void* implementation, unsigned int version)
{
    iroLog("new resource ",this, " with id ", id, " and type ", (interface) ? interface->name : "<unknown>", ", version ", version, " for wl_client ", &client);

    wlResource_ = wl_resource_create(&client, interface, version, id);
    if(!wlResource_)
    {
        //todo: really throw here?? its a runtime function
        std::string error = "failed to create resource for " + std::string(interface->name) + ", id " + std::to_string(id) + ", version " + std::to_string(version);
        throw std::runtime_error(error);
        return;
    }

    wl_resource_set_implementation(wlResource_, implementation, this, destroyResource);
}

wl_client& resource::getWlClient() const
{
    return *(wl_resource_get_client(wlResource_));
}

client& resource::getClient() const
{
    return iroCompositor()->getClient(getWlClient());
}

unsigned int resource::getID() const
{
    return wl_resource_get_id(wlResource_);
}

//////////////////
bool operator==(const resource& r1, const resource& r2)
{
    return (r1.getID() == r2.getID() && &r1.getWlClient() == &r2.getWlClient());
}
