#include <iro/compositor/xdgSurface.hpp>
#include <protos/wayland-xdg-shell-server-protocol.h>

namespace iro
{

//c interface implementation
void xdgSurfaceDestroy(wl_client* client, wl_resource* resource)
{
}
void xdgSurfaceSetParent(wl_client* client, wl_resource* resource, wl_resource* parent)
{
}
void xdgSurfaceSetTitle(wl_client* client, wl_resource* resource, const char* title)
{
}
void xdgSurfaceSetAppId(wl_client* client, wl_resource* resource, const char* app_id)
{
}
void xdgSurfaceShowWindowMenu(wl_client* client, wl_resource* resource, wl_resource* seat, 
		uint32_t serial, int32_t x, int32_t y)
{
}
void xdgSurfaceMove(wl_client* client, wl_resource* resource, wl_resource* seat, uint32_t serial)
{
}
void xdgSurfaceResize(wl_client* client, wl_resource* resource, wl_resource* seat, uint32_t serial,
	   	uint32_t edges)
{
}
void xdgSurfaceAckConfigure(wl_client* client, wl_resource* resource, uint32_t serial)
{
}
void xdgSurfaceSetWindowGeometry(wl_client* client, wl_resource* resource, int32_t x, int32_t y,
	int32_t width, int32_t height)
{
}
void xdgSurfaceSetMaximized(wl_client* client, wl_resource* resource)
{
}
void xdgSurfaceUnsetMaximized(wl_client* client, wl_resource* resource)
{
}
void xdgSurfaceSetFullscreen(wl_client* client, wl_resource* resource, wl_resource* output)
{
}
void xdgSurfaceUnsetFullscreen(wl_client* client, wl_resource* resource)
{
}
void xdgSurfaceSetMinimized(wl_client* client, wl_resource* resource)
{
}

const struct xdg_surface_interface xdgSurfaceImplementation = 
{
	&xdgSurfaceDestroy,
	&xdgSurfaceSetParent,
	&xdgSurfaceSetTitle,
	&xdgSurfaceSetAppId,
	&xdgSurfaceShowWindowMenu,
	&xdgSurfaceMove,
	&xdgSurfaceResize,
	&xdgSurfaceAckConfigure,
	&xdgSurfaceSetWindowGeometry,
	&xdgSurfaceSetMaximized,
	&xdgSurfaceUnsetMaximized,
	&xdgSurfaceSetFullscreen,
	&xdgSurfaceUnsetFullscreen,
	&xdgSurfaceSetMinimized
};

//class implementation
XdgSurfaceRes::XdgSurfaceRes(SurfaceRes& surface, wl_client& client, unsigned int id)
	: Resource(client, id, &xdg_surface_interface, &xdgSurfaceImplementation, 1)
{
}

}
