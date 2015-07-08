#include <compositor/subcompositor.hpp>

#include <resources/surface.hpp>
#include <compositor/compositor.hpp>

void subcompositorDestroy(wl_client* client, wl_resource* resource)
{
    subcompositorRes* res = (subcompositorRes*) wl_resource_get_user_data(resource);
    delete res;
}
void subcompositorGetSubsurface(wl_client* client, wl_resource* resource, unsigned int id, wl_resource* surface, wl_resource* parent)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(surface);
    surfaceRes* par = (surfaceRes*) wl_resource_get_user_data(parent);
    surf->setSubsurface(id, par);
}
const struct wl_subcompositor_interface subcompositorImplementation =
{
    &subcompositorDestroy,
    &subcompositorGetSubsurface
};
void bindSubcompositor(wl_client* client, void* data, unsigned int version, unsigned int id)
{
    new subcompositorRes(client, id, version);
}

/////////////////////////////////
subcompositor::subcompositor()
{
    wl_global_create(getCompositor()->getWlDisplay(), &wl_subcompositor_interface, 1, this, bindSubcompositor);
}

/////////////////////////
subcompositorRes::subcompositorRes(wl_client* client, unsigned int id, unsigned int version) : resource(client, id, &wl_subcompositor_interface, &subcompositorImplementation, version)
{

}
