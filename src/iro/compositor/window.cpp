#include <iro/compositor/window.hpp>
#include <iro/compositor/compositor.hpp>
#include <iro/seat/seat.hpp>
#include <iro/seat/pointer.hpp>
#include <iro/seat/keyboard.hpp>
#include <iro/seat/event.hpp>
#include <iro/backend/output.hpp>

#include <nytl/enumOps.hpp>
using namespace nytl::enumOps;

#include <wayland-server-protocol.h>

//todo: events move/resize
// -care about callback connection destroy/release grabs on window destruction

namespace iro
{

void Window::startMove(Seat& seat, const Event& trigger)
{
	if(!seat.pointer())
	{
		nytl::sendWarning("Window::Move: given seat has no pointer");
		return;
	}

	Pointer::Grab grab;
	grab.exclusive = 1;
	grab.moveFunction = [=](const nytl::vec2i&, const nytl::vec2i& delta)
			{
				this->move(delta);	
			};

	auto* bev = static_cast<const PointerButtonEvent*>(&trigger);
	if(bev)
	{
		auto captureEvent = *bev;
		grab.buttonFunction = 
			[this, captureEvent, &seat](unsigned int button, bool press)
			{
				if(button == captureEvent.button && press != captureEvent.state) 
					seat.pointer()->releaseGrab();
			};
	}
	else
	{
		nytl::sendWarning("Window::Move: invalid event type");
		return;
	}

	seat.pointer()->grab(grab);
	states_ |= State::moving;
}

void Window::startResize(Seat& seat, const Event& trigger, unsigned int edges)
{
	if(!seat.pointer())
	{
		nytl::sendWarning("Window::Resize: given seat has no pointer");
		return;
	}

	Pointer::Grab grab;
	grab.exclusive = 1;
	grab.moveFunction = [=](const nytl::vec2i&, const nytl::vec2i& delta)
			{
				this->resize(delta, edges);	
			};

	auto* bev = static_cast<const PointerButtonEvent*>(&trigger);
	if(bev)
	{
		auto captureEvent = *bev;
		grab.buttonFunction = 
			[this, captureEvent, &seat](unsigned int button, bool press)
			{
				if(button == captureEvent.button && press != captureEvent.state) 
					seat.pointer()->releaseGrab();
			};
	}
	else
	{
		nytl::sendWarning("Window::Resize: invalid event type");
		return;
	}

	seat.pointer()->grab(grab);
	states_ |= State::resizing;
}

void Window::startMove(Seat& seat, unsigned int triggerSerial)
{
	auto* ev = seat.compositor().event(triggerSerial);
	if(!ev)
	{
		nytl::sendWarning("Window::Move: invalid serial");
		return;
	}

	startMove(seat, *ev);
}

void Window::startResize(Seat& seat, unsigned int triggerSerial, unsigned int edges)
{
	auto* ev = seat.compositor().event(triggerSerial);
	if(!ev)
	{
		nytl::sendWarning("Window::Resize: invalid serial");
		return;
	}

	startResize(seat, *ev, edges);
}

void Window::move(const nytl::vec2i& delta)
{
	position_ += delta;
}

void Window::resize(const nytl::vec2i& delta, unsigned int edges)
{
	if(edges & WL_SHELL_SURFACE_RESIZE_LEFT)
	{
		pending_.geometry_.size.x -= delta.x;
		position_.x += delta.x;	
	}
	else if(edges & WL_SHELL_SURFACE_RESIZE_RIGHT)
	{
		pending_.geometry_.size.x += delta.x;
	}
	if(edges & WL_SHELL_SURFACE_RESIZE_TOP)
	{
		pending_.geometry_.size.y -= delta.y;
		position_.y += delta.y;	
	}
	if(edges & WL_SHELL_SURFACE_RESIZE_BOTTOM)
	{
		pending_.geometry_.size.y += delta.y;
	}

	sendConfigure(pending_.geometry_.size);
}

void Window::showWindowMenu(Seat& seat, const Event& trigger, const nytl::vec2i& position)
{
}

void Window::showWindowMenu(Seat& seat, unsigned int triggerSerial, const nytl::vec2i& position)
{
	auto* ev = seat.compositor().event(triggerSerial);
	if(!ev)
	{
		nytl::sendWarning("Window::showMenu: invalid serial");
		return;
	}

	showWindowMenu(seat, *ev, position);
}

bool Window::mapped() const
{
	return ((!parent_ || parent_->mapped()) && 
			!static_cast<bool>(states_ & State::minimized)); 
}

void Window::commit()
{
	commited_ = pending_;
}

void Window::geometry(const nytl::rect2i& geo)
{
	pending_.geometry_ = geo;
}

void Window::normalState()
{
	states_ &= ~State::maximized | ~State::minimized | ~State::fullscreen | ~State::popup | 
		~State::transient;
}

void Window::setMaximized(Output& outp)
{
	normalState();
	states_ |= State::maximized;

	position_ = {0, 0};
	sendConfigure(outp.size());
}
void Window::setFullscreen(Output& outp, unsigned int method)
{
	normalState();
	states_ |= State::fullscreen;

	position_ = {0, 0};
	sendConfigure(outp.size());
}
void Window::setMinimized()
{
	normalState();
	states_ |= State::minimized;


}
void Window::setTransient(SurfaceRes& parent, const nytl::vec2i& position, unsigned int flags)
{
	normalState();
	states_ |= State::transient;
}
void Window::setPopup(SurfaceRes& parent, const nytl::vec2i& pos, unsigned int flags, Seat& seat)
{
	normalState();
	states_ |= State::popup;
}
void Window::setNormal()
{
	normalState();
	states_ |= State::normal;

	sendConfigure(normalSize_);
}

void Window::sendConfigure(const nytl::vec2ui&) const
{
}

}
