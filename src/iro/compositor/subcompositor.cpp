#include <iro/compositor/subcompositor.hpp>

#include <iro/compositor/surface.hpp>
#include <iro/compositor/client.hpp>
#include <iro/compositor/compositor.hpp>
#include <iro/compositor/subsurface.hpp>

#include <nytl/make_unique.hpp>
#include <ny/base/log.hpp>

#include <wayland-server-protocol.h>

namespace iro
{

//resource
class SubcompositorRes : public Resource
{
public:
    SubcompositorRes(wl_client& client, unsigned int id, unsigned int version);

    virtual unsigned int type() const { return resourceType::subcompositor; }
};

namespace
{

//wayland implementation
void subcompositorDestroy(wl_client*, wl_resource* resource)
{
	SubcompositorRes* subres = Resource::validateDisconnect<SubcompositorRes>(resource, 
			"subcompositorDestroy");
	if(!subres) return;

	subres->destroy();
}
void subcompositorGetSubsurface(wl_client* client, wl_resource* resource, 
		unsigned int id, wl_resource* surface, wl_resource* parent)
{
	SurfaceRes* surf = Resource::validateDisconnect<SurfaceRes>(surface, "getsubsurf1");
	SurfaceRes* parentSurf = Resource::validateDisconnect<SurfaceRes>(parent, "getsubsurf2");
    if(!surf || !parentSurf) return; 

	if(surf->role())
	{
		ny::sendWarning("subcompGetSubsurf: surface already has a role");
		wl_resource_post_error(resource, WL_SUBCOMPOSITOR_ERROR_BAD_SURFACE, "alrady has a role");
		return;
	}

	auto subres = nytl::make_unique<SubsurfaceRes>(*surf, *client, id, *parentSurf);
	surf->role(nytl::make_unique<SubsurfaceRole>(*subres));
	surf->client().addResource(std::move(subres));
}
const struct wl_subcompositor_interface subcompositorImplementation =
{
    &subcompositorDestroy,
    &subcompositorGetSubsurface
};
void bindSubcompositor(wl_client* client, void* data, unsigned int version, unsigned int id)
{
	Subcompositor* subcomp = static_cast<Subcompositor*>(data);
	if(!subcomp)
	{
		ny::sendWarning("bindSubcompositor: invalid data");
		return;
	}

	auto& clnt = subcomp->compositor().client(*client);
	auto scRes = nytl::make_unique<SubcompositorRes>(*client, id, version);
	clnt.addResource(std::move(scRes));
}

}

//Subcompositor
Subcompositor::Subcompositor(Compositor& comp) : compositor_(&comp)
{
    wlGlobal_ = wl_global_create(&comp.wlDisplay(), &wl_subcompositor_interface, 1, 
			this, bindSubcompositor);
}

Subcompositor::~Subcompositor()
{
}

//SubcompositorRes
SubcompositorRes::SubcompositorRes(wl_client& client, unsigned int id, unsigned int version) 
	: Resource(client, id, &wl_subcompositor_interface, &subcompositorImplementation, version)
{
}

}
