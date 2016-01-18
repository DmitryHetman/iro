#include <iro/compositor/xdgSurface.hpp>
#include <iro/compositor/compositor.hpp>
#include <iro/compositor/shell.hpp>
#include <iro/backend/output.hpp>
#include <iro/backend/backend.hpp>
#include <iro/seat/seat.hpp>
#include <iro/seat/pointer.hpp>
#include <iro/seat/event.hpp>
#include <iro/util/wlArray.hpp>

#include <protos/wayland-xdg-shell-server-protocol.h>

#include <nytl/enumOps.hpp>
using namespace nytl::enumOps;

#include <wayland-server-protocol.h>

#include <nytl/log.hpp>
#include <nytl/make_unique.hpp>

namespace iro
{

//XdgConfigureEvent
namespace eventType
{
	constexpr unsigned int XdgConfigure = 101;
};

class XdgConfigureEvent : public Event
{
public:
	XdgConfigureEvent(Client* c = nullptr) : Event(eventType::XdgConfigure, c) {}
};
	
//c interface implementation
void xdgSurfaceDestroy(wl_client*, wl_resource* resource)
{
	auto* xsurfRes = Resource::validateDisconnect<XdgSurfaceRes>(resource, "xdgSurfaceDestroy");
	if(!xsurfRes) return;

	xsurfRes->destroy();
}
void xdgSurfaceSetParent(wl_client* client, wl_resource* resource, wl_resource* parent)
{
}
void xdgSurfaceSetTitle(wl_client* client, wl_resource* resource, const char* title)
{
	auto* xsurfRes = Resource::validateDisconnect<XdgSurfaceRes>(resource, "xdgSurfaceTitle");
	if(!xsurfRes) return;

	xsurfRes->title(title);
}
void xdgSurfaceSetAppId(wl_client* client, wl_resource* resource, const char* app_id)
{
	auto* xsurfRes = Resource::validateDisconnect<XdgSurfaceRes>(resource, "xdgSurfaceAppID");
	if(!xsurfRes) return;

	xsurfRes->appID(app_id);
}
void xdgSurfaceShowWindowMenu(wl_client* client, wl_resource* resource, wl_resource* seat, 
		uint32_t serial, int32_t x, int32_t y)
{
	auto* xsurfRes = Resource::validateDisconnect<XdgSurfaceRes>(resource, "xdgSurfaceAppID");
	auto* seatRes = Resource::validateDisconnect<SeatRes>(seat, "xdgSurfaceAppID");
	if(!xsurfRes || !seatRes) return;

	xsurfRes->showWindowMenu(seatRes->seat(), serial, {x, y});
}
void xdgSurfaceMove(wl_client*, wl_resource* resource, wl_resource* seat, uint32_t serial)
{
	auto* xsurfRes = Resource::validateDisconnect<XdgSurfaceRes>(resource, "xdgSurfaceMove");
	auto* seatRes = Resource::validateDisconnect<SeatRes>(seat, "xdgSurfaceMove2");
	if(!seatRes || !xsurfRes) return;

	xsurfRes->startMove(seatRes->seat(), serial);
}
void xdgSurfaceResize(wl_client*, wl_resource* resource, wl_resource* seat, uint32_t serial,
	   	uint32_t edges)
{
	auto* xsurfRes = Resource::validateDisconnect<XdgSurfaceRes>(resource, "xdgSurfaceResize");
	auto* seatRes = Resource::validateDisconnect<SeatRes>(seat, "xdgSurfaceResize2");
	if(!seatRes || !xsurfRes) return;

	xsurfRes->startResize(seatRes->seat(), serial, edges);
}
void xdgSurfaceAckConfigure(wl_client* client, wl_resource* resource, uint32_t serial)
{
}
void xdgSurfaceSetWindowGeometry(wl_client*, wl_resource* resource, int32_t x, int32_t y,
	int32_t width, int32_t height)
{
	auto* xsurfRes = Resource::validateDisconnect<XdgSurfaceRes>(resource, "xdgSurfaceGeometry");
	if(!xsurfRes) return;

	xsurfRes->geometry(nytl::rect2i({x,y}, {width, height}));
}
void xdgSurfaceSetMaximized(wl_client* client, wl_resource* resource)
{
	auto* xsurfRes = Resource::validateDisconnect<XdgSurfaceRes>(resource, "xdgSurfaceGeometry");
	if(!xsurfRes) return;

	xsurfRes->setMaximized();
}
void xdgSurfaceUnsetMaximized(wl_client* client, wl_resource* resource)
{
	auto* xsurfRes = Resource::validateDisconnect<XdgSurfaceRes>(resource, "xdgSurfaceGeometry");
	if(!xsurfRes) return;

	xsurfRes->setNormal();
}
void xdgSurfaceSetFullscreen(wl_client* client, wl_resource* resource, wl_resource* output)
{
	auto* xsurfRes = Resource::validateDisconnect<XdgSurfaceRes>(resource, "xdgSurfaceGeometry");
	if(!xsurfRes) return;

	xsurfRes->setFullscreen();
}
void xdgSurfaceUnsetFullscreen(wl_client* client, wl_resource* resource)
{
	auto* xsurfRes = Resource::validateDisconnect<XdgSurfaceRes>(resource, "xdgSurfaceGeometry");
	if(!xsurfRes) return;

	xsurfRes->setNormal();
}
void xdgSurfaceSetMinimized(wl_client* client, wl_resource* resource)
{
	auto* xsurfRes = Resource::validateDisconnect<XdgSurfaceRes>(resource, "xdgSurfaceGeometry");
	if(!xsurfRes) return;

	xsurfRes->setMinimized();
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
XdgSurfaceRes::XdgSurfaceRes(SurfaceRes& surf, wl_client& client, unsigned int id, unsigned int v)
	: Resource(client, id, &xdg_surface_interface, &xdgSurfaceImplementation, v), 
		surface_(&surf)
{
	auto shell = compositor().shell();
	if(shell) shell->windowCreated(*this);
}

XdgSurfaceRes::~XdgSurfaceRes()
{
	auto shell = compositor().shell();
	if(shell) shell->windowDestroyed(*this);
}

void XdgSurfaceRes::close()
{
	xdg_surface_send_close(&wlResource());
}

void XdgSurfaceRes::xdgStates(wl_array& arr) const
{
	if(static_cast<bool>(states() & State::activated))
		wlArrayPush<uint32_t>(arr, XDG_SURFACE_STATE_ACTIVATED);
	if(static_cast<bool>(states() & State::fullscreen))
		wlArrayPush<uint32_t>(arr, XDG_SURFACE_STATE_FULLSCREEN);
	if(static_cast<bool>(states() & State::maximized))
		wlArrayPush<uint32_t>(arr, XDG_SURFACE_STATE_MAXIMIZED);
	if(static_cast<bool>(states() & State::resizing))
		wlArrayPush<uint32_t>(arr, XDG_SURFACE_STATE_RESIZING);
}

void XdgSurfaceRes::sendConfigure(const nytl::vec2ui& size) const
{
	wl_array states;
	wl_array_init(&states);
	xdgStates(states);

	auto& ev = compositor().event(nytl::make_unique<XdgConfigureEvent>(), 1);
	xdg_surface_send_configure(&wlResource(), size.x, size.y, &states, ev.serial);
}

void XdgSurfaceRes::setMaximized()
{
	if(!compositor().backend()) return;

	setMaximized(*compositor().backend()->outputs()[0]);
}

void XdgSurfaceRes::setFullscreen()
{
	if(!compositor().backend()) return;

	setFullscreen(*compositor().backend()->outputs()[0],WL_SHELL_SURFACE_FULLSCREEN_METHOD_DEFAULT);
}

}
