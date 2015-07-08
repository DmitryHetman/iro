#pragma once

#include <iro.hpp>
#include <resources/resource.hpp>
#include <util/region.hpp>

#include <wayland-server-core.h>

class regionRes : public resource
{
protected:
    region region_;

public:
    regionRes(wl_client* client, unsigned int id);

    region& getRegion(){ return region_; }
    const region& getregion() const { return region_; }

    resourceType getType() const { return resourceType::region; }
};
