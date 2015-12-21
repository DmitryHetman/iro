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
	nytl::vec2i hotspot_;

public:
	CursorSurfaceRole(Pointer& ptr) : pointer_(&ptr) {}

	virtual unsigned int roleType() const override { return surfaceRoleType::cursor; }
	virtual nytl::vec2i position() const override;
	virtual bool mapped() const override { return 1; }
	virtual void commit() override {}

	const nytl::vec2i& hotspot() const { return hotspot_; }
	void hotspot(const nytl::vec2i& hs) { hotspot_ = hs; }

	Pointer& pointer() const { return *pointer_; }
};

}
