#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>
#include <iro/compositor/surface.hpp>
#include <iro/compositor/window.hpp>

#include <nytl/vec.hpp>
#include <nytl/rect.hpp>

namespace iro
{

///Represents a client xdg-surface resource.
class XdgSurfaceRes : public Resource, public Window
{
protected:
	SurfaceRes* surface_;

protected:
	virtual void sendConfigure(const nytl::vec2ui& size) const override;
	void xdgStates(wl_array& states) const;

public:
	XdgSurfaceRes(SurfaceRes& surface, wl_client& client, unsigned int id, unsigned int v);
	~XdgSurfaceRes();

	using Window::setMaximized;
	using Window::setFullscreen;

	void setMaximized();
	void setFullscreen();

	virtual void close() override;
	virtual SurfaceRes* surfaceRes() const override { return surface_; }
};

///The surface role for a xdg surface.
class XdgSurfaceRole : public WindowSurfaceRole
{
public:
	XdgSurfaceRole(XdgSurfaceRes& res) : WindowSurfaceRole(res) {};

    virtual unsigned int roleType() const override { return surfaceRoleType::xdgSurface; }
	XdgSurfaceRes& xdgSurfaceRes() const { return static_cast<XdgSurfaceRes&>(window()); }
};

}
