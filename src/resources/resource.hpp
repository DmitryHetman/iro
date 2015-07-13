#pragma once

#include <iro.hpp>
#include <util/nonCopyable.hpp>

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

    callback,

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

    resource() = default;
    resource(wl_client* client, unsigned int id, const struct wl_interface* interface, const void* implementation, unsigned int version = 1, void* data = nullptr, void(*destroyFunc)(wl_resource*) = nullptr);

    void create(wl_client* client, unsigned int id, const struct wl_interface* interface, const void* implementation, unsigned int version = 1, void* data = nullptr, void(*destroyFunc)(wl_resource*) = nullptr);

public:
    resource(wl_resource* res);
    virtual ~resource();

    void destroy();

    unsigned int getID() const;
    unsigned int getVersion() const;

    wl_resource* getWlResource() const { return wlResource_; }
    wl_client* getWlClient() const;
    client* getClient() const;

    virtual resourceType getType() const { return resourceType::unknown; };
};
