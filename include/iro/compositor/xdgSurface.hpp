#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>
#include <iro/compositor/surface.hpp>

#include <nytl/vec.hpp>

namespace iro
{

///Represents a client xdg-surface resource.
class XdgSurfaceRes : public Resource
{
protected:
	SurfaceRes* surface_;
	nytl::vec2i position_ {0, 0};

public:
	XdgSurfaceRes(SurfaceRes& surface, wl_client& client, unsigned int id);
	
	const nytl::vec2i& position() const { return position_; }
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
    virtual void commit() override {} //todo
    virtual unsigned int roleType() const override { return surfaceRoleType::xdgSurface; }
};

}
