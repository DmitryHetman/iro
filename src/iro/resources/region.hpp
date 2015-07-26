#pragma once

#include <iro/include.hpp>
#include <iro/resources/resource.hpp>
#include <nyutil/region.hpp>

class regionRes : public resource
{
protected:
    region region_;

public:
    regionRes(wl_client& client, unsigned int id);

    region& getRegion(){ return region_; }
    const region& getRegion() const { return region_; }

    resourceType getType() const { return resourceType::region; }
};
