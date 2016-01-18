#include <iro/compositor/shellSurface.hpp>
#include <iro/compositor/compositor.hpp>
#include <iro/backend/output.hpp>
#include <iro/seat/seat.hpp>
#include <iro/seat/pointer.hpp>
#include <iro/seat/keyboard.hpp>
#include <iro/seat/event.hpp>

#include <nytl/log.hpp>
#include <nytl/make_unique.hpp>

#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

namespace iro
{

namespace
{

void shellSurfacePong(wl_client*, wl_resource* res, unsigned int serial)
{
    auto* ssurfRes = Resource::validateDisconnect<ShellSurfaceRes>(res, "shellsurfacepong");
	if(!ssurfRes) return;

    ssurfRes->pong(serial);
}
void shellSurfaceMove(wl_client*, wl_resource* res, wl_resource* seat, 
		unsigned int serial)
{
	auto* ssurfRes = Resource::validateDisconnect<ShellSurfaceRes>(res, "shellSurfaceMove");
	auto* seatRes = Resource::validateDisconnect<SeatRes>(seat, "shellSurfaceMove2");
	if(!seatRes || !ssurfRes) return;

	ssurfRes->startMove(seatRes->seat(), serial);
}
void shellSurfaceResize(wl_client*, wl_resource* res, wl_resource* seat, 
		unsigned int serial, unsigned int edges)
{
	auto* ssurfRes = Resource::validateDisconnect<ShellSurfaceRes>(res, "shellSurfaceResize");
	auto* seatRes = Resource::validateDisconnect<SeatRes>(seat, "shellSurfaceResize2");
	if(!seatRes || !ssurfRes) return;

	ssurfRes->startResize(seatRes->seat(), serial, edges);
}
void shellSurfaceSetToplevel(wl_client*, wl_resource* res)
{
	auto* ssurfRes = Resource::validateDisconnect<ShellSurfaceRes>(res, "shellSurfaceToplevel");
	if(!ssurfRes) return;

	ssurfRes->setNormal();
}

void shellSurfaceSetTransient(wl_client*, wl_resource* res, wl_resource* parent, 
		int x, int y, unsigned int flags)
{
	auto* ssurfRes = Resource::validateDisconnect<ShellSurfaceRes>(res, "shellSurfaceTransient");
	auto* parentRes = Resource::validateDisconnect<SurfaceRes>(parent, "shellSurfaceTransient2");
	if(!ssurfRes || !parentRes) return;

	ssurfRes->setTransient(*parentRes, {x, y}, flags);
}

void shellSurfaceSetFullscreen(wl_client*, wl_resource* res, unsigned int method,
	   	unsigned int framerate, wl_resource* output)
{
	auto* ssurfRes = Resource::validateDisconnect<ShellSurfaceRes>(res, "shellSurfaceFullscreen");
	auto* outputRes = Resource::validateDisconnect<OutputRes>(output, "shellSurfaceFullscreen2");
	if(!ssurfRes || !outputRes) return;

	ssurfRes->setFullscreen(outputRes->output(), method); //framrate?
}

void shellSurfaceSetPopup(wl_client*, wl_resource* res, wl_resource* seat, 
		unsigned int serial, wl_resource* parent, int x, int y, unsigned int flags)
{
	auto* ssurfRes = Resource::validateDisconnect<ShellSurfaceRes>(res, "shellSurfacePopup");
	auto* parentRes = Resource::validateDisconnect<SurfaceRes>(parent, "shellSurfacePopup2");
	auto* seatRes = Resource::validateDisconnect<SeatRes>(seat, "shellSurfacePopup3");
	if(!ssurfRes || !parentRes || !seatRes) return;

	//check serial!
	ssurfRes->setPopup(*parentRes, {x, y}, flags, seatRes->seat());
}
void shellSurfaceSetMaximized(wl_client*, wl_resource* res, wl_resource* output)
{
	auto* ssurfRes = Resource::validateDisconnect<ShellSurfaceRes>(res, "shellSurfaceFullscreen");
	auto* outputRes = Resource::validateDisconnect<OutputRes>(output, "shellSurfaceFullscreen2");
	if(!ssurfRes || !outputRes) return;

	ssurfRes->setMaximized(outputRes->output());
}
void shellSurfaceSetTitle(wl_client*, wl_resource* res, const char* title)
{
	auto* ssurfRes = Resource::validateDisconnect<ShellSurfaceRes>(res, "shellSurfacePopup");
	if(!ssurfRes) return;

    ssurfRes->title(title);
}
void shellSurfaceSetClass(wl_client*, wl_resource* res, const char* className)
{
	auto* ssurfRes = Resource::validateDisconnect<ShellSurfaceRes>(res, "shellSurfacePopup");
	if(!ssurfRes) return;

    ssurfRes->appID(className);
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

}

//
ShellSurfaceRes::ShellSurfaceRes(SurfaceRes& surf, wl_client& client, unsigned int id) 
	: Resource(client, id, &wl_shell_surface_interface, &shellSurfaceImplementation), 
		surface_(&surf)
{
}

unsigned int ShellSurfaceRes::ping()
{
	auto& ev = compositor().event(nytl::make_unique<PingEvent>(), 1);

    pingSerial_ = ev.serial;
	pingTime_ = nytl::now();
    wl_shell_surface_send_ping(&wlResource(), pingSerial_);

	return pingSerial_;
}

void ShellSurfaceRes::pong(unsigned int serial)
{
	if(serial != pingSerial_)
	{
		nytl::sendWarning("ShellSurfaceRes::pong: serials do not match\n\t",
				"this: ", this, " pingSerial_: ", pingSerial_, " serial: ", serial);
		return;
	}

	pingSerial_ = 0;
}

}
