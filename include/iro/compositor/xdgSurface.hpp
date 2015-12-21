#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>
#include <iro/compositor/surface.hpp>

#include <nytl/vec.hpp>
#include <nytl/rect.hpp>

namespace iro
{

///Represents a client xdg-surface resource.
class XdgSurfaceRes : public Resource
{
protected:
	SurfaceRes* surface_;
	nytl::vec2i position_ {0, 0};

	nytl::rect2i geometry_ = {0, 0, 0, 0};
	nytl::rect2i geometryPending_ = {0, 0, 0, 0};

	unsigned int resizeEdges_;

	void move(const nytl::vec2i& delta);
	void resize(const nytl::vec2i& delta);

public:
	XdgSurfaceRes(SurfaceRes& surface, wl_client& client, unsigned int id, unsigned int v);

	void beginMove(SeatRes& seat, unsigned int serial);
	void beginResize(SeatRes& seat, unsigned int serial, unsigned int edges);

	void setGeometry(const nytl::rect2i& geometry);
	void commit();
	
	const nytl::vec2i& position() const { return position_; }
	SurfaceRes& surface() const { return *surface_; }
};

///The surface role for a xdg surface.
class XdgSurfaceRole : public SurfaceRole
{
protected:
	XdgSurfaceRes* xdgSurfaceRes_;

public:
	XdgSurfaceRole(XdgSurfaceRes& res) : xdgSurfaceRes_(&res) {};

    virtual nytl::vec2i position() const override { return xdgSurfaceRes_->position(); }
    virtual bool mapped() const override { return 1; } //todo?
    virtual void commit() override { xdgSurfaceRes_->commit(); } //todo
    virtual unsigned int roleType() const override { return surfaceRoleType::xdgSurface; }

	XdgSurfaceRes& xdgSurfaceRes() const { return *xdgSurfaceRes_; }
};

}
