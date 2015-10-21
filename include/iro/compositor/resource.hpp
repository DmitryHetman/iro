#pragma once

#include <iro/include.hpp>
#include <nyutil/nonCopyable.hpp>
#include <nyutil/callback.hpp>
#include <nyutil/misc.hpp>

//#include <type_traits>
#include <iostream>

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
friend void destroyResource(wl_resource*);

protected:
    wl_resource* wlResource_ = nullptr;
    callback<void(resource&)> destructionCallback_;

    resource() = default;
    resource(wl_client& client, unsigned int id, const wl_interface* interface, const void* implementation, unsigned int version = 1);

    void create(wl_client& client, unsigned int id, const wl_interface* interface, const void* implementation, unsigned int version = 1);

    virtual ~resource();

public:
    resource(wl_resource& res);

    virtual void destroy();

    unsigned int getID() const;
    unsigned int getVersion() const;

    wl_resource& getWlResource() const { return *wlResource_; }
    wl_client& getWlClient() const;
    client& getClient() const;

    //resources can be custom destroyed [resources::destroy] or are automatically destroyed when their wl_resource object is destroyed -> wl_resource should be ALWAYS valid
    std::unique_ptr<connection> onDestruction(std::function<void()> func){ return destructionCallback_.add([=](resource&){func();}); }
    std::unique_ptr<connection> onDestruction(std::function<void(resource&)> func){ return destructionCallback_.add(func); }

    //restype
    virtual resourceType getType() const { return resourceType::unknown; };
};

bool operator==(const resource& r1, const resource& r2);


/////
//todo: make copyable with valid operators/constructors
template<typename T> class resourceRef : public nonCopyable /*: std::enable_if<std::is_base_of<resource, T>::type>*/
{
protected:
    T* resource_ = nullptr;
    std::unique_ptr<connection> resourceConnection_ = nullptr;

    void destroyCB()
    {
        resource_ = nullptr;
        resourceConnection_ = nullptr;
    }

public:
    resourceRef() = default;
    resourceRef(T& res){ set(&res); }
    resourceRef(T* res){ if(res) set(&res); }
    ~resourceRef()
    {
        if(resourceConnection_) resourceConnection_->destroy();
    }

    resourceRef(resourceRef&& other) noexcept : resource_(other.resource_), resourceConnection_(std::move(other.resourceConnection_)) { other.resourceConnection_.reset(); }
    resourceRef<T>& operator=(resourceRef&& other) noexcept { resource_ = other.resource_; resourceConnection_ = std::move(other.resourceConnection_); other.resourceConnection_.reset(); return *this; }

    T* get() const { return resource_; }
    void set(T* value)
    {
        if(resource_)
        {
            resourceConnection_->destroy();
            resourceConnection_.reset();
        }

        if(value)
        {
            resourceConnection_ = value->onDestruction(memberCallback(&resourceRef<T>::destroyCB, this)); //its ok if T ids unknown
        }

        resource_ = value;
    }
};
