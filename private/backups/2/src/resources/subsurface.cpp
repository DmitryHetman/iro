#include <resources/subsurface.hpp>

#include <resources/surface.hpp>

void subsurfaceDestroy(wl_client* client, wl_resource* resource)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    surf->unsetRole();
}
void subsurfaceSetPosition(wl_client* client, wl_resource* resource, int x, int y)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    surf->getSubsurface()->setPosition(vec2ui(x,y));
}
void subsurfacePlaceAbove(wl_client* client, wl_resource* resource, wl_resource* sibling)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    surfaceRes* sib = (surfaceRes*) wl_resource_get_user_data(sibling);

    if(surf->isChild(sib) || surf->getSubsurface()->getParent() == sib)
    {
        surf->getPending().zOrder = sib->getCommited().zOrder + 1;
    }
    else
    {
        wl_resource_post_error(resource, WL_SUBSURFACE_ERROR_BAD_SURFACE, "subsurface place above can only be called with parent or sibling surface");
    }
}
void subsurfacePlaceBelow(wl_client* client, wl_resource* resource, wl_resource* sibling)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    surfaceRes* sib = (surfaceRes*) wl_resource_get_user_data(sibling);

    if(surf->isChild(sib) || surf->getSubsurface()->getParent() == sib)
    {
        surf->getPending().zOrder = sib->getCommited().zOrder - 1;
    }
    else
    {
        wl_resource_post_error(resource, WL_SUBSURFACE_ERROR_BAD_SURFACE, "subsurface place below can only be called with parent or sibling surface");
    }
}
void subsurfaceSetSync(wl_client* client, wl_resource* resource)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    surf->getSubsurface()->setSync(1);
}
void subsurfaceSetDesync(wl_client* client, wl_resource* resource)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    surf->getSubsurface()->setSync(0);
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
subsurfaceRes::subsurfaceRes(surfaceRes* surface, wl_client* client, unsigned int id, surfaceRes* parent) : resource(client, id, &wl_subsurface_interface, &subsurfaceImplementation, 1, surface), parent_(parent)
{
}
