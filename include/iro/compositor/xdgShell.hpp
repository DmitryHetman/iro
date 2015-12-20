#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>
#include <iro/util/global.hpp>

#include <nytl/vec.hpp>
#include <nytl/time.hpp>

namespace iro
{

///Represents a xdg-shell from the xdg-shell wayland extension.
class XdgShell : public Global
{
protected:
	Compositor* compositor_;

public:
	XdgShell(Compositor& comp);
	virtual ~XdgShell() = default;

	virtual void getXdgSurface(SurfaceRes& surface, unsigned int id);
	virtual void getXdgPopup(SurfaceRes& surface, unsigned int id, SurfaceRes& parent, 
			SeatRes& seat, unsigned int serial, const nytl::vec2i& position);

	Compositor& compositor() const { return *compositor_; }
};

///Holds a clients XdgShell resource.
class XdgShellRes : public Resource
{
protected:
	XdgShell* xdgShell_;

	unsigned int pingSerial_ = -1;
	nytl::timePoint pingTime_;

public:
	XdgShellRes(XdgShell& shell, wl_client& client, unsigned int id, unsigned int v);

	XdgShell& xdgShell() const { return *xdgShell_; }

	unsigned int ping();
	void pong(unsigned int serial);

	bool activePing() const { return (pingSerial_ != 0); }
	nytl::timeDuration pingTime() const { return nytl::now() - pingTime_; }
};

}
