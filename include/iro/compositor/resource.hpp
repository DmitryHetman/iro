#pragma once

#include <iro/include.hpp>
#include <nyutil/nonCopyable.hpp>
#include <nyutil/callback.hpp>

enum class resourceType : unsigned char
{
    unknown = 0,

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
    callback<void()> destructionCallback_;

    resource() = default;
    resource(wl_client& client, unsigned int id, const wl_interface* interface, const void* implementation, unsigned int version = 1);

    void create(wl_client& client, unsigned int id, const wl_interface* interface, const void* implementation, unsigned int version = 1);

public:
    resource(wl_resource& res);
    virtual ~resource();

    virtual void destroy();

    unsigned int getID() const;
    unsigned int getVersion() const;

    wl_resource& getWlResource() const { return *wlResource_; }
    wl_client& getWlClient() const;
    client& getClient() const;

    connection& onDestruct(std::function<void()> func){ return destructionCallback_.add(func); }

    //restype
    virtual resourceType getType() const { return resourceType::unknown; };
};

bool operator==(const resource& r1, const resource& r2);
