#include <iro/shell/shell.hpp>
#include <iro/iro.hpp>
#include <iro/resources/surface.hpp>
#include <iro/resources/shellSurface.hpp>
#include <iro/log.hpp>

#include <wayland-server-protocol.h>


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

void shell::setModule(iroShellModule& mod)
{
    if(module_)
    {
        getIro()->unloadModule(*module_);
    }

    module_ = &mod;
}

void shell::addExtension(iroShellExtension& ext)
{
    extensions_.push_back(&ext);
}

void shell::removeExtension(iroShellExtension& ext)
{
    for(auto it = extensions_.begin(); it != extensions_.end(); ++it)
    {
        if(&ext == *it)
        {
            getIro()->unloadModule(**it);

            extensions_.erase(it);
            return;
        }
    }
}

/////////////////////////////
shellRes::shellRes(wl_client& client, unsigned int id, unsigned int version) : resource(client, id, &wl_shell_interface, &shellImplementation, version)
{

}
