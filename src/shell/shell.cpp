#include <shell/shell.hpp>

#include <resources/surface.hpp>
#include <resources/shellSurface.hpp>

#include <wayland-server-protocol.h>

#include <log.hpp>

//shell
void shelliroShellSurface(wl_client* client, wl_resource* resource, unsigned int id, wl_resource* surface)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(surface);
    surf->setShellSurface(id);
}
const struct wl_shell_interface shellImplementation =
{
    &shelliroShellSurface
};

void bindShell(wl_client* client, void* data, unsigned int version, unsigned int id)
{
    new shellRes(*client, id, version);
}


////////////////////////////
shell::shell()
{
    global_ = wl_global_create(iroWlDisplay(), &wl_shell_interface, 1, this, bindShell);
}

shell::~shell()
{
    //wl_global_destroy(global_);
}

/////////////////////////////
shellRes::shellRes(wl_client& client, unsigned int id, unsigned int version) : resource(client, id, &wl_shell_interface, &shellImplementation, version)
{

}
