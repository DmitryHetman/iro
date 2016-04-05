#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>
#include <nytl/rectRegion.hpp>

namespace iro
{

///Resource that represents a region (basically a vector of rectangles). 
///Internally it uses the nytl::region2i class.
class RegionRes : public Resource
{
protected:
	nytl::RectRegion2i region_;

public:
    RegionRes(wl_client& client, unsigned int id);

	nytl::RectRegion2i& region(){ return region_; }
    const nytl::RectRegion2i& region() const { return region_; }
};

}
