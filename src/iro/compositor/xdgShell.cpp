#include <iro/compositor/xdgShell.hpp>
#include <iro/compositor/resource.hpp>
#include <iro/compositor/compositor.hpp>
#include <iro/compositor/client.hpp>
#include <iro/compositor/xdgSurface.hpp>
#include <iro/compositor/xdgPopup.hpp>
#include <iro/seat/seat.hpp>
#include <iro/seat/event.hpp>

#include <protos/wayland-xdg-shell-server-protocol.h>

#include <nytl/make_unique.hpp>
#include <nytl/log.hpp>

namespace iro
{


//xdg_shell-interface implementation
void xdgShellDestroy(wl_client*, wl_resource* resource)
{
	auto* xshellres = Resource::validateDisconnect<XdgShellRes>(resource, "xShellSurface");
	if(!xshellres) return;

	xshellres->destroy();
}

void xdgShellUseUnstable(wl_client* client, wl_resource* resource, int32_t version)
{
	//TODO
}

void xdgShellGetSurface(wl_client*, wl_resource* resource, uint32_t id, wl_resource* surface)
{
	auto* xshellres = Resource::validateDisconnect<XdgShellRes>(resource, "xShellSurface");
	auto* surf = Resource::validateDisconnect<SurfaceRes>(surface, "xShellSurface2");
	if(!xshellres || !surf) return;

	xshellres->xdgShell().getXdgSurface(*surf, id, xshellres->version());
}

void xdgShellGetPopup(wl_client* client, wl_resource* resource, uint32_t id, wl_resource* surface,
	wl_resource* parent, wl_resource* seat, uint32_t serial, int32_t x, int32_t y)
{
	auto* xshellres = Resource::validateDisconnect<XdgShellRes>(resource, "xShellSurface");
	auto* surf = Resource::validateDisconnect<SurfaceRes>(surface, "xShellSurface2");
	auto* parentR = Resource::validateDisconnect<SurfaceRes>(parent, "xShellSurface3");
	auto* seatR = Resource::validateDisconnect<SeatRes>(seat, "xShellSurface4");
	if(!xshellres || !surf || !parentR || !seatR) return;

	xshellres->xdgShell().getXdgPopup(*surf, id,* parentR,* seatR, serial, {x,y},
			xshellres->version());
}

void xdgShellPong(wl_client* client, wl_resource* resource, uint32_t serial)
{
	auto* xshellres = Resource::validateDisconnect<XdgShellRes>(resource, "xShellSurface");
	if(!xshellres) return;

	xshellres->pong(serial);
}

const struct xdg_shell_interface xdgShellImpl = 
{
	&xdgShellDestroy,
	&xdgShellUseUnstable,
	&xdgShellGetSurface,
	&xdgShellGetPopup,
	&xdgShellPong
};

void bindXdgShell(wl_client* client, void* data, unsigned int version, unsigned int id)
{
	XdgShell* xdgShell = static_cast<XdgShell*>(data);
	if(!xdgShell) return;

    auto& clnt = xdgShell->compositor().client(*client);
	clnt.addResource(nytl::make_unique<XdgShellRes>(*xdgShell, *client, id, version));
}

//XdgShell
XdgShell::XdgShell(Compositor& comp) : compositor_(&comp)
{
	wlGlobal_ = wl_global_create(&comp.wlDisplay(), &xdg_shell_interface, 1, this, bindXdgShell);
	if(!wlGlobal_)
	{
        throw std::runtime_error("XdgShell::XdgShell: failed to create wayland global");
	}
}

void XdgShell::getXdgSurface(SurfaceRes& surf, unsigned int id, unsigned int version)
{
	auto xdgSurfaceRes = nytl::make_unique<XdgSurfaceRes>(surf, surf.wlClient(), id, version);
	auto xdgSurfaceRole = nytl::make_unique<XdgSurfaceRole>(*xdgSurfaceRes);

	surf.client().addResource(std::move(xdgSurfaceRes));
	surf.role(std::move(xdgSurfaceRole));
}

void XdgShell::getXdgPopup(SurfaceRes& surface, unsigned int id, SurfaceRes& parent, SeatRes& seat,
		unsigned int serial, const nytl::vec2i& position, unsigned int version)
{
	nytl::sendLog("xdgpopup");
	auto xdgPopupRes = nytl::make_unique<XdgPopupRes>(surface, parent, id);
	auto xdgPopupRole = nytl::make_unique<XdgPopupRole>(*xdgPopupRes);

	surface.client().addResource(std::move(xdgPopupRes));
	surface.role(std::move(xdgPopupRole));
}

//XdgShellRes
XdgShellRes::XdgShellRes(XdgShell& shell, wl_client& client, unsigned int id, unsigned int version)
	: Resource(client, id, &xdg_shell_interface, &xdgShellImpl, version), xdgShell_(&shell)
{
}

unsigned int XdgShellRes::ping()
{
	auto& ev = compositor().event(nytl::make_unique<PingEvent>(), 1);

    pingSerial_ = ev.serial;
	pingTime_ = nytl::now();
    xdg_shell_send_ping(&wlResource(), pingSerial_);

	return pingSerial_;
}

void XdgShellRes::pong(unsigned int serial)
{
	if(serial != pingSerial_)
	{
		nytl::sendWarning("XdgShellRes::pong: serials do not match\n\t",
				"this: ", this, " pingSerial_: ", pingSerial_, " serial: ", serial);
		return;
	}

	pingSerial_ = 0;
}

}
