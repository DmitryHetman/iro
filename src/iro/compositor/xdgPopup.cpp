#include <iro/compositor/xdgPopup.hpp>
#include <protos/wayland-xdg-shell-server-protocol.h>

namespace iro
{

namespace
{

void xdgPopupDestroy(wl_client*, wl_resource* resource)
{
	auto* popupres = Resource::validateDisconnect<XdgPopupRes>(resource, "xdgPopupDestroy");
	if(!popupres) return;

	popupres->destroy();
}

const struct xdg_popup_interface xdgPopupImplementation = 
{
	&xdgPopupDestroy
};

}

//
XdgPopupRes::XdgPopupRes(SurfaceRes& surface, SurfaceRes& parent, unsigned int id) 
	: Resource(surface.wlClient(), id, &xdg_popup_interface, &xdgPopupImplementation, 1), 
	surface_(&surface), parent_(&parent)
{
}

}
