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

	ssurfRes->beginMove(*seatRes, serial);
}
void shellSurfaceResize(wl_client*, wl_resource* res, wl_resource* seat, 
		unsigned int serial, unsigned int edges)
{
	auto* ssurfRes = Resource::validateDisconnect<ShellSurfaceRes>(res, "shellSurfaceResize");
	auto* seatRes = Resource::validateDisconnect<SeatRes>(seat, "shellSurfaceResize2");
	if(!seatRes || !ssurfRes) return;

	ssurfRes->beginResize(*seatRes, serial, edges);
}
void shellSurfaceSetToplevel(wl_client*, wl_resource* res)
{
	auto* ssurfRes = Resource::validateDisconnect<ShellSurfaceRes>(res, "shellSurfaceToplevel");
	if(!ssurfRes) return;

	ssurfRes->setToplevel();
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

	ssurfRes->setFullscreen(method, framerate, *outputRes);
}

void shellSurfaceSetPopup(wl_client*, wl_resource* res, wl_resource* seat, 
		unsigned int serial, wl_resource* parent, int x, int y, unsigned int flags)
{
	auto* ssurfRes = Resource::validateDisconnect<ShellSurfaceRes>(res, "shellSurfacePopup");
	auto* parentRes = Resource::validateDisconnect<SurfaceRes>(parent, "shellSurfacePopup2");
	auto* seatRes = Resource::validateDisconnect<SeatRes>(seat, "shellSurfacePopup3");
	if(!ssurfRes || !parentRes || !seatRes) return;

	ssurfRes->setPopup(*seatRes, serial, *parentRes, {x, y}, flags);
}
void shellSurfaceSetMaximized(wl_client*, wl_resource* res, wl_resource* output)
{
	auto* ssurfRes = Resource::validateDisconnect<ShellSurfaceRes>(res, "shellSurfaceFullscreen");
	auto* outputRes = Resource::validateDisconnect<OutputRes>(output, "shellSurfaceFullscreen2");
	if(!ssurfRes || !outputRes) return;

	ssurfRes->setMaximized(*outputRes);
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

    ssurfRes->className(className);
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

//
ShellSurfaceRes::ShellSurfaceRes(SurfaceRes& surf, wl_client& client, unsigned int id) 
	: Resource(client, id, &wl_shell_surface_interface, &shellSurfaceImplementation), surface_(surf)
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

void ShellSurfaceRes::beginMove(SeatRes& seat, unsigned int serial)
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
		nytl::sendWarning("ShellSurfaceMove: invalid event type");
		return;
	}

	seat.seat().pointer()->grab(myGrab);
}

void ShellSurfaceRes::beginResize(SeatRes& seat, unsigned int serial, unsigned int edges)
{
	//todo: check current state, destroy callbacks/grabs
	Seat* s = &seat.seat();
	if(!seat.seat().pointer()) return;

	Event* ev = seat.seat().compositor().event(serial);
	if(!ev)
	{
		//todo: warn or even kick client here
		nytl::sendWarning("ShellSurfaceResize: invalid serial");
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
		nytl::sendWarning("ShellSurfaceResize: invalid event type");
		return;
	}

	resizeEdges_ = edges;
	seat.seat().pointer()->grab(myGrab);
}

void ShellSurfaceRes::setToplevel()
{
}
void ShellSurfaceRes::setTransient(SurfaceRes& parent, const nytl::vec2i& pos, unsigned int flags)
{
}
void ShellSurfaceRes::setFullscreen(unsigned int mthd, unsigned int frames, OutputRes& output)
{
}
void ShellSurfaceRes::setPopup(SeatRes& seat, unsigned int serial, SurfaceRes& parent, 
		const nytl::vec2i& pos, unsigned int flags)
{
}
void ShellSurfaceRes::setMaximized(OutputRes& output)
{
}

void ShellSurfaceRes::move(const nytl::vec2i& delta)
{
	position_ += delta;
}

void ShellSurfaceRes::resize(const nytl::vec2i& delta)
{

}

}
