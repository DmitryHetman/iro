#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>
#include <iro/compositor/surface.hpp>

namespace iro
{

class XdgPopupRes : public Resource
{
protected:
	SurfaceRes* surface_;
	SurfaceRes* parent_;

public:
	XdgPopupRes(SurfaceRes& surface, SurfaceRes& parent, unsigned int id);
};

class XdgPopupRole : public SurfaceRole
{
protected:
	XdgPopupRes* xdgPopupRes_;

public:
	XdgPopupRole(XdgPopupRes& popup) : xdgPopupRes_(&popup) {}

	virtual nytl::Vec2i position() const override { return {}; }
	virtual void commit() override {}
	virtual bool mapped() const override { return 1; }
	virtual unsigned int roleType() const override { return surfaceRoleType::xdgPopup; }
};

}
