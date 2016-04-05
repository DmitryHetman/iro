#pragma once

#include <iro/include.hpp>
#include <iro/compositor/surface.hpp>
#include <nytl/vec.hpp>

namespace iro
{

///Represents a cursor role for a surface.
class CursorSurfaceRole : public SurfaceRole
{
protected:
	Pointer* pointer_;
	SurfaceRes* surface_;
	nytl::Vec2i hotspot_;

public:
	CursorSurfaceRole(Pointer& ptr, SurfaceRes& surf) 
		: pointer_(&ptr), surface_(&surf) {}

	virtual unsigned int roleType() const override { return surfaceRoleType::cursor; }
	virtual nytl::Vec2i position() const override;
	virtual bool mapped() const override;
	virtual void commit() override {}

	const nytl::Vec2i& hotspot() const { return hotspot_; }
	void hotspot(const nytl::Vec2i& hs) { hotspot_ = hs; }

	Pointer& pointer() const { return *pointer_; }
	SurfaceRes& surfaceRes() const { return *surface_; }
};

}
