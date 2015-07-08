#include <shell/shell.hpp>

#include <compositor/compositor.hpp>
#include <resources/surface.hpp>
#include <resources/shellSurface.hpp>

//shell
void shellGetShellSurface(wl_client* client, wl_resource* resource, unsigned int id, wl_resource* surface)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(surface);
    surf->setShellSurface(id);
}
const struct wl_shell_interface shellImplementation =
{
    &shellGetShellSurface
};

void bindShell(wl_client* client, void* data, unsigned int version, unsigned int id)
{
    new shellRes(client, id, version);
}


////////////////////////////
shell::shell()
{
    wl_global_create(getCompositor()->getWlDisplay(), &wl_shell_interface, 1, this, bindShell);
}

shell::~shell()
{

}

/////////////////////////////
shellRes::shellRes(wl_client* client, unsigned int id, unsigned int version) : resource(client, id, &wl_shell_interface, &shellImplementation, version)
{

}
