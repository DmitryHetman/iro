#include <iro/compositor/region.hpp>
#include <wayland-server-protocol.h>


namespace iro
{

//wayland implementation
void regionDestroy(wl_client*, wl_resource* resource)
{
	RegionRes* rs = Resource::validateDisconnect<RegionRes>(resource, "regionDestroy");
	if(!rs) return;

    rs->destroy();
}
void regionAdd(wl_client*, wl_resource* resource, int x, int y, int width, int height)
{
	RegionRes* rs = Resource::validateDisconnect<RegionRes>(resource, "regionAdd");
	if(!rs) return;

    rs->region().add(nytl::rect2i(x, y, width, height));
}
void regionSubtract(wl_client*, wl_resource* resource, int x, int y, int width, int height)
{
	RegionRes* rs = Resource::validateDisconnect<RegionRes>(resource, "regionSubtract");
	if(!rs) return;

    rs->region().subtract(nytl::rect2i(x, y, width, height));
}

const struct wl_region_interface regionImplementation
{
    &regionDestroy,
    &regionAdd,
    &regionSubtract
};

//regionRes implementation
RegionRes::RegionRes(wl_client& client, unsigned int id) 
	: Resource(client, id, &wl_region_interface, &regionImplementation)
{
}

}
