#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>
#include <nytl/region.hpp>

namespace iro
{

///Resource that represents a region (basically a vector of rectangles). 
///Internally it uses the nytl::region2i class.
class RegionRes : public Resource
{
protected:
	nytl::region2i region_;

public:
    RegionRes(wl_client& client, unsigned int id);

	nytl::region2i& region(){ return region_; }
    const nytl::region2i& region() const { return region_; }
};

}
