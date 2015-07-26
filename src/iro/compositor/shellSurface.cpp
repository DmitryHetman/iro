#include <iro/compositor/shellSurface.hpp>
#include <iro/seat/seat.hpp>

#include <wayland-server-protocol.h>

void shellSurfacePong(wl_client* client, wl_resource* resource, unsigned int serial)
{
    shellSurfaceRes* surf = (shellSurfaceRes*) wl_resource_get_user_data(resource);
    surf->pong();
}
void shellSurfaceMove(wl_client* client, wl_resource* resource, wl_resource* seat, unsigned int serial)
{
    seatRes* seatr = (seatRes*) wl_resource_get_user_data(seat);
    shellSurfaceRes* surf = (shellSurfaceRes*) wl_resource_get_user_data(resource);

    seatr->getSeat().moveShellSurface(seatr, surf);
}
void shellSurfaceResize(wl_client* client, wl_resource* resource, wl_resource* seat, unsigned int serial, unsigned int edges)
{
    seatRes* seatr = (seatRes*) wl_resource_get_user_data(seat);
    shellSurfaceRes* surf = (shellSurfaceRes*) wl_resource_get_user_data(resource);

    seatr->getSeat().resizeShellSurface(seatr, surf, edges);
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
    shellSurfaceRes* surf = (shellSurfaceRes*) wl_resource_get_user_data(resource);
    surf->setClassName(title);
}
void shellSurfaceSetClass(wl_client* client, wl_resource* resource, const char* className)
{
    shellSurfaceRes* surf = (shellSurfaceRes*) wl_resource_get_user_data(resource);
    surf->setClassName(className);
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
shellSurfaceRes::shellSurfaceRes(surfaceRes& surf, wl_client& client, unsigned int id) : resource(client, id, &wl_shell_surface_interface, &shellSurfaceImplementation), surface_(surf)
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

void shellSurfaceRes::ping()
{
    ping_ = 1;
    wl_shell_surface_send_ping(&getWlResource(), wl_display_next_serial(iroWlDisplay()));
}
