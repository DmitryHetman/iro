#include <resources/shellSurface.hpp>

#include <wayland-server-protocol.h>

void shellSurfacePong(wl_client* client, wl_resource* resource, unsigned int serial)
{

}
void shellSurfaceMove(wl_client* client, wl_resource* resource, wl_resource* seat, unsigned int serial)
{

}
void shellSurfaceResize(wl_client* client, wl_resource* resource, wl_resource* seat, unsigned int serial, unsigned int edges)
{

}
void shellSurfaceSetToplevel(wl_client* client, wl_resource* resource)
{

}
void shellSurfaceSetTransient(wl_client* client, wl_resource* resource, wl_resource* parent, int x, int y, unsigned int flags)
{

}
void shellSurfaceSetFullscreen(wl_client* client, wl_resource* resource, unsigned int method, unsigned int framerate, wl_resource* output)
{

}
void shellSurfaceSetPopup(wl_client* client, wl_resource* resource, wl_resource* seat, unsigned int serial, wl_resource* parent, int x, int y, unsigned int flags)
{

}
void shellSurfaceSetMaximized(wl_client* client, wl_resource* resource, wl_resource* output)
{

}
void shellSurfaceSetTitle(wl_client* client, wl_resource* resource, const char* title)
{

}
void shellSurfaceSetClass(wl_client* client, wl_resource* resource, const char* class_)
{

}

const struct wl_shell_surface_interface shellSurfaceImplementation
{
    &shellSurfacePong,
    &shellSurfaceMove,
    &shellSurfaceResize,
    &shellSurfaceSetToplevel,
    &shellSurfaceSetTransient,
    &shellSurfaceSetFullscreen,
    &shellSurfaceSetPopup,
    &shellSurfaceSetMaximized,
    &shellSurfaceSetTitle,
    &shellSurfaceSetClass
};

///////////////////////////////////////////////////////////////////////
shellSurfaceRes::shellSurfaceRes(surfaceRes* surf, wl_client* client, unsigned int id) : resource(client, id, &wl_shell_surface_interface, &shellSurfaceImplementation), surface_(surf)
{
}

void shellSurfaceRes::setClassName(const std::string& name)
{
    className_ = name;
}

void shellSurfaceRes::setTitle(const std::string& name)
{
    title_ = name;
}
