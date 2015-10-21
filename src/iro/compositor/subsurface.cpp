#include <iro/compositor/subsurface.hpp>

#include <iro/compositor/surface.hpp>
#include <wayland-server-protocol.h>

void subsurfaceDestroy(wl_client* client, wl_resource* resource)
{
    subsurfaceRes* surf = (subsurfaceRes*) wl_resource_get_user_data(resource);
    delete surf;
}
void subsurfaceSetPosition(wl_client* client, wl_resource* resource, int x, int y)
{
    subsurfaceRes* surf = (subsurfaceRes*) wl_resource_get_user_data(resource);
    surf->setPosition(vec2ui(x,y));
}
void subsurfacePlaceAbove(wl_client* client, wl_resource* resource, wl_resource* sibling)
{
    //todo
}
void subsurfacePlaceBelow(wl_client* client, wl_resource* resource, wl_resource* sibling)
{
    //todo
}
void subsurfaceSetSync(wl_client* client, wl_resource* resource)
{
    subsurfaceRes* surf = (subsurfaceRes*) wl_resource_get_user_data(resource);
    surf->setSync(1);
}
void subsurfaceSetDesync(wl_client* client, wl_resource* resource)
{
    subsurfaceRes* surf = (subsurfaceRes*) wl_resource_get_user_data(resource);
    surf->setSync(0);
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
subsurfaceRes::subsurfaceRes(surfaceRes& surf, wl_client& client, unsigned int id, surfaceRes& parent) : resource(client, id, &wl_subsurface_interface, &subsurfaceImplementation), surface_(surf), parent_(parent)
{
}
