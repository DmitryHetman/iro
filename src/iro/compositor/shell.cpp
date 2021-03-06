#include <iro/compositor/shell.hpp>
#include <iro/compositor/compositor.hpp>
#include <iro/compositor/resource.hpp>
#include <iro/compositor/client.hpp>
#include <iro/compositor/shellSurface.hpp>

#include <ny/base/log.hpp>

#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

namespace iro
{

//ShellRes
class ShellRes : public Resource
{
protected:
	Shell* shell_;

public:
	ShellRes(Shell& shell, wl_client& client, unsigned int id, unsigned int v);

	Shell& shell() const { return *shell_; }
};

namespace
{

void shellGetShellSurface(wl_client*, wl_resource* shell, unsigned int id, wl_resource* surf)
{
	auto surface = Resource::validateDisconnect<SurfaceRes>(surf, "shellGetShellSurface");
	if(!surface) return;	

	auto shellR = Resource::validateDisconnect<ShellRes>(shell, "shellGetShellSurface2");
	if(!shellR) return;

	shellR->shell().getShellSurface(*surface, id);
}

const struct wl_shell_interface shellImplementation = 
{
	&shellGetShellSurface
};

void bindShell(wl_client* client, void* data, unsigned int version, unsigned int id)
{
	Shell* shell = static_cast<Shell*>(data);
	if(!shell) return;

    auto& clnt = shell->compositor().client(*client);
	clnt.addResource(std::make_unique<ShellRes>(*shell, *client, id, version));
}

}

//ShellRes constructor
ShellRes::ShellRes(Shell& shell, wl_client& clnt, unsigned int id, unsigned int v) 
	: Resource(clnt, id, &wl_shell_interface, &shellImplementation, v), shell_(&shell) {}

///
Shell::Shell(Compositor& comp) : compositor_(&comp)
{
	wlGlobal_ = wl_global_create(&comp.wlDisplay(), &wl_shell_interface, 1, this, bindShell);
	if(!wlGlobal_)
	{
        throw std::runtime_error("Shell::Shell: failed to create wayland global");
	}
}

void Shell::getShellSurface(SurfaceRes& surface, unsigned int id)
{
	auto shellSurfaceRes = std::make_unique<ShellSurfaceRes>(surface, surface.wlClient(), id);
	auto shellSurfaceRole = std::make_unique<ShellSurfaceRole>(*shellSurfaceRes);

	surface.client().addResource(std::move(shellSurfaceRes));
	surface.role(std::move(shellSurfaceRole));
}

}
