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

    ssurf->position({x,y});
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

//SubsurfaceResObserver
void SubsurfaceRes::SurfaceObserver::destructionCallback(nytl::Observable& surface)
{
	if(static_cast<SurfaceRes*>(&surface) == subsurface_->surface_)
	{
		subsurface_->surface_ = nullptr;
	}
	else if(static_cast<SurfaceRes*>(&surface) == subsurface_->parent_)
	{
		subsurface_->parent_ = nullptr;
		//need something like this in surface since it gets unmapped without a commit
		//if(surface_) surface->updateMap();
	}
};

//SubsurfaceRes
SubsurfaceRes::SubsurfaceRes(SurfaceRes& surf, wl_client& client, unsigned int id, 
	SurfaceRes& parent) : Resource(client, id, &wl_subsurface_interface, &subsurfaceImplementation),
	surface_(&surf), parent_(&parent)
{
	//listen to surface destruction
	surfaceObserver_.subsurface_ = this;
	parentObserver_.subsurface_ = this;

	surface_->addObserver(surfaceObserver_);
	parent_->addObserver(parentObserver_);
	std::cout << "observer1 " << &parentObserver_ << "\n";
	std::cout << "observer2 " << &surfaceObserver_ << "\n";
}

SubsurfaceRes::~SubsurfaceRes()
{
	if(surface())
	{
		surface()->clearRole();
		surface()->removeObserver(surfaceObserver_);
	}

	if(parent())
	{
		parent()->removeObserver(parentObserver_);
	}
}

void SubsurfaceRes::commit()
{
	//TODO
}

}
