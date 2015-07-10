#pragma once

#include <iro.hpp>
#include <resources/resource.hpp>

#include <util/nonCopyable.hpp>

///////////////////////////////////////////
class compositor : public nonCopyable
{
protected:
    wl_display* wlDisplay_ = nullptr;

    backend* backend_ = nullptr;
    shell* shell_ = nullptr;
    seat* seat_ = nullptr;
    subcompositor* subcompositor_ = nullptr;

    static compositor* object;

public:
    compositor();
    ~compositor();

    int run();

    backend* getBackend() const { return backend_; }
    shell* getShell() const { return shell_; }
    seat* getSeat() const { return seat_; }
    subcompositor* getSubcompositor() const { return subcompositor_; }

    wl_display* getWlDisplay() const { return wlDisplay_; }
    wl_event_loop* getWlEventLoop() const;

    static compositor* getObject(){ return object; }
};

//////////////////////////
class compositorRes : public resource
{
public:
    compositorRes(wl_client* client, unsigned int id, unsigned int version);

    resourceType getType() const { return resourceType::compositor; }
};
