#include <iro/compositor/xdgSurface.hpp>
#include <iro/compositor/compositor.hpp>
#include <iro/backend/output.hpp>
#include <iro/seat/seat.hpp>
#include <iro/seat/pointer.hpp>
#include <iro/seat/event.hpp>
#include <protos/wayland-xdg-shell-server-protocol.h>

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
	auto* xsurfRes = Resource::validateDisconnect<XdgSurfaceRes>(resource, "xdgSurfaceMove");
	if(!xsurfRes) return;

	xsurfRes->destroy();
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
void xdgSurfaceMove(wl_client*, wl_resource* resource, wl_resource* seat, uint32_t serial)
{
	auto* xsurfRes = Resource::validateDisconnect<XdgSurfaceRes>(resource, "xdgSurfaceMove");
	auto* seatRes = Resource::validateDisconnect<SeatRes>(seat, "xdgSurfaceMove2");
	if(!seatRes || !xsurfRes) return;

	xsurfRes->beginMove(*seatRes, serial);
}
void xdgSurfaceResize(wl_client*, wl_resource* resource, wl_resource* seat, uint32_t serial,
	   	uint32_t edges)
{
	auto* xsurfRes = Resource::validateDisconnect<XdgSurfaceRes>(resource, "xdgSurfaceResize");
	auto* seatRes = Resource::validateDisconnect<SeatRes>(seat, "xdgSurfaceResize2");
	if(!seatRes || !xsurfRes) return;

	xsurfRes->beginResize(*seatRes, serial, edges);
}
void xdgSurfaceAckConfigure(wl_client* client, wl_resource* resource, uint32_t serial)
{
	std::cout << "AckConfigure: " << serial << "\n";
}
void xdgSurfaceSetWindowGeometry(wl_client*, wl_resource* resource, int32_t x, int32_t y,
	int32_t width, int32_t height)
{
	auto* xsurfRes = Resource::validateDisconnect<XdgSurfaceRes>(resource, "xdgSurfaceGeometry");
	if(!xsurfRes) return;

	xsurfRes->setGeometry(nytl::rect2i({x,y}, {width, height}));
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
XdgSurfaceRes::XdgSurfaceRes(SurfaceRes& surf, wl_client& client, unsigned int id, unsigned int v)
	: Resource(client, id, &xdg_surface_interface, &xdgSurfaceImplementation, v), 
		surface_(&surf)
{
}

void XdgSurfaceRes::beginMove(SeatRes& seat, unsigned int serial)
{
	//todo: check current state, destroy callbacks/grabs
	nytl::sendLog("shellsurfaceRes: begin move");

	Seat* s = &seat.seat();
	if(!seat.seat().pointer()) return;

	Event* ev = seat.seat().compositor().event(serial);
	if(!ev)
	{
		//todo: warn or even kick client here
		nytl::sendWarning("ShellSurfaceMove: invalid serial");
		return;
	}	

	//release grab on destruction! store the status somewhere
	Pointer::Grab myGrab;
	myGrab.exclusive = 1;
	myGrab.moveFunction = [=](const nytl::vec2i&, const nytl::vec2i& delta)
			{
				this->move(delta);	
			};

	if(ev->type == eventType::pointerButton)
	{
		auto* bEv = static_cast<PointerButtonEvent*>(ev);
		myGrab.buttonFunction = [=](unsigned int button, bool press)
				{
					if(button == bEv->button && press != bEv->state) 
						s->pointer()->releaseGrab();
				};
	}
	else
	{
		nytl::sendWarning("XdgSurfaceRes::beginMove: invalid event type");
		return;
	}

	seat.seat().pointer()->grab(myGrab);
}

void XdgSurfaceRes::beginResize(SeatRes& seat, unsigned int serial, unsigned int edges)
{
	//todo: check current state, destroy callbacks/grabs
	Seat* s = &seat.seat();
	if(!seat.seat().pointer()) return;

	Event* ev = seat.seat().compositor().event(serial);
	if(!ev)
	{
		//todo: warn or even kick client here
		nytl::sendWarning("XdgSurfaceRes::beginResize: invalid serial");
		return;
	}	

	//release grab on destruction! store the status somewhere
	Pointer::Grab myGrab;
	myGrab.exclusive = 1;
	myGrab.moveFunction = [=](const nytl::vec2i&, const nytl::vec2i& delta)
			{
				this->resize(delta);	
			};

	if(ev->type == eventType::pointerButton)
	{
		auto* bEv = static_cast<PointerButtonEvent*>(ev);
		myGrab.buttonFunction = [=](unsigned int button, bool press)
				{
					if(button == bEv->button && press != bEv->state) 
						s->pointer()->releaseGrab();


				};
	}
	else
	{
		nytl::sendWarning("XdgSurfaceRes::beginResize: invalid event type");
		return;
	}

	resizeEdges_ = edges;
	seat.seat().pointer()->grab(myGrab);
}

void XdgSurfaceRes::move(const nytl::vec2i& delta)
{
	position_ += delta;
}

void XdgSurfaceRes::resize(const nytl::vec2i& delta)
{
	/*
	geometry_.size += delta;
	auto& ev = compositor().event(nytl::make_unique<XdgConfigureEvent>(&client()), 1);

	wl_array states;
	wl_array_init(&states);
	unsigned int* ptr = static_cast<unsigned int*>(wl_array_add(&states, 4));
	*ptr = XDG_SURFACE_STATE_RESIZING;

	std::cout << "geo: " << geometry_ << " delta: " << delta <<"\n";
	xdg_surface_send_configure(&wlResource(), geometry_.position.x, geometry_.position.y, 
			&states, ev.serial);
			*/
}

void XdgSurfaceRes::setGeometry(const nytl::rect2i& geometry)
{
	std::cout << "geo: " << geometry_ << "\n";
	geometryPending_ = geometry;
}

void XdgSurfaceRes::commit()
{
	geometry_ = geometryPending_;
}

}
