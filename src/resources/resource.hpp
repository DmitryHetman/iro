#pragma once

#include <iro.hpp>
#include <util/nonCopyable.hpp>

#include <wayland-server-protocol.h>

enum class resourceType : unsigned char
{
    unknown,

    surface,
    shellSurface,
    subsurface,
    region,
    pointer,
    keyboard,
    touch,
    buffer,
    dataDevice,
    dataSource,
    dataOffer,

    //globals
    seat,
    compositor,
    subcompositor,
    output,
    shell,
    dataDeviceManager
};

class resource : public nonCopyable
{
protected:
    wl_resource* wlResource_ = nullptr;

    resource(wl_resource* res) : wlResource_(res) {};
    resource(wl_client* client, unsigned int id, const struct wl_interface* interface, const void* implementation, unsigned int version = 1, void* data = nullptr, wl_resource_destroy_func_t destroyFunc = nullptr);
    resource() = default;

    void create(wl_client* client, unsigned int id, const struct wl_interface* interface, const void* implementation, unsigned int version = 1, void* data = nullptr, wl_resource_destroy_func_t destroyFunc = nullptr);

public:
    virtual ~resource();
    void destroy();

    unsigned int getID() const;
    unsigned int getVersion() const;

    wl_resource* getWlResource() const { return wlResource_; }
    wl_client* getWlClient() const;

    virtual resourceType getType() const { return resourceType::unknown; };
};
