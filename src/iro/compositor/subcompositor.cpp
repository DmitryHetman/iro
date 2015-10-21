#include <iro/compositor/subcompositor.hpp>

#include <iro/compositor/surface.hpp>
#include <iro/compositor/compositor.hpp>
#include <iro/compositor/subsurface.hpp>

#include <wayland-server-protocol.h>

void subcompositorDestroy(wl_client* client, wl_resource* resource)
{
    subcompositorRes* res = (subcompositorRes*) wl_resource_get_user_data(resource);
    delete res;
}
void subcompositorGetSubsurface(wl_client* client, wl_resource* resource, unsigned int id, wl_resource* surface, wl_resource* parent)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(surface);
    surfaceRes* par = (surfaceRes*) wl_resource_get_user_data(parent);

    if(!surf || !par) return; //error?

    new subsurfaceRes(*surf, *client, id, *par);
}
const struct wl_subcompositor_interface subcompositorImplementation =
{
    &subcompositorDestroy,
    &subcompositorGetSubsurface
};
void bindSubcompositor(wl_client* client, void* data, unsigned int version, unsigned int id)
{
    new subcompositorRes(*client, id, version);
}

/////////////////////////////////
subcompositor::subcompositor()
{
    global_ = wl_global_create(iroWlDisplay(), &wl_subcompositor_interface, 1, this, bindSubcompositor);
}

subcompositor::~subcompositor()
{
    wl_global_destroy(global_);
}
/////////////////////////
subcompositorRes::subcompositorRes(wl_client& client, unsigned int id, unsigned int version) : resource(client, id, &wl_subcompositor_interface, &subcompositorImplementation, version)
{

}
