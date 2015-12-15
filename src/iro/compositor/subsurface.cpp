#include <iro/compositor/subsurface.hpp>

#include <iro/compositor/surface.hpp>
#include <wayland-server-protocol.h>

namespace iro
{

void subsurfaceDestroy(wl_client*, wl_resource* resource)
{
	SubsurfaceRes* ssurf = Resource::validateDisconnect<SubsurfaceRes>(resource, "subsurfdestroy");
	if(!ssurf) return;

	ssurf->destroy();
}
void subsurfaceSetPosition(wl_client*, wl_resource* resource, int x, int y)
{
	SubsurfaceRes* ssurf = Resource::validateDisconnect<SubsurfaceRes>(resource, "subsurfpos");
	if(!ssurf) return;

    ssurf->position(nytl::vec2ui(x,y));
}
void subsurfacePlaceAbove(wl_client* client, wl_resource* resource, wl_resource* sibling)
{
    //todo
}
void subsurfacePlaceBelow(wl_client* client, wl_resource* resource, wl_resource* sibling)
{
    //todo
}
void subsurfaceSetSync(wl_client*, wl_resource* resource)
{
	SubsurfaceRes* ssurf = Resource::validateDisconnect<SubsurfaceRes>(resource, "subsurfsync");
	if(!ssurf) return;

    ssurf->synced(1);
}
void subsurfaceSetDesync(wl_client*, wl_resource* resource)
{
	SubsurfaceRes* ssurf = Resource::validateDisconnect<SubsurfaceRes>(resource, "subsurfdesync");
	if(!ssurf) return;

    ssurf->synced(0);
}
const struct wl_subsurface_interface subsurfaceImplementation =
{
    &subsurfaceDestroy,
    &subsurfaceSetPosition,
    &subsurfacePlaceAbove,
    &subsurfacePlaceBelow,
    &subsurfaceSetSync,
    &subsurfaceSetDesync
};

/////////////////////////////7
SubsurfaceRes::SubsurfaceRes(SurfaceRes& surf, wl_client& client, unsigned int id, 
		SurfaceRes& parent) 
	: Resource(client, id, &wl_subsurface_interface, 
		&subsurfaceImplementation), surface_(surf), parent_(parent)
{
}

}
