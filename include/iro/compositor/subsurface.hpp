#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>
#include <iro/compositor/surface.hpp>

#include <nytl/vec.hpp>
#include <nytl/callback.hpp>

namespace iro
{

///SubsurfaceRes represents a subsurface resource as well as the subsurface role for a surface.
class SubsurfaceRes : public Resource
{
protected:
	friend class SurfaceObserver;
	class SurfaceObserver : public nytl::Observer 
	{
	public:
		SubsurfaceRes* subsurface_;
		virtual void destructionCallback(nytl::Observable& surface) override;
	};

protected:
    SurfaceRes* surface_;
    SurfaceRes* parent_;
    bool sync_ = 0;

	nytl::Vec2i position_;

	SurfaceObserver surfaceObserver_;
	SurfaceObserver parentObserver_;

public:
    SubsurfaceRes(SurfaceRes& surface, wl_client& client, unsigned int id, SurfaceRes& parent);
	~SubsurfaceRes();

    SurfaceRes* surface() const { return surface_; };
    SurfaceRes* parent() const { return parent_; }

    bool synced() const { return sync_; }
    void synced(bool sync){ sync_ = sync; }

    void position(const nytl::Vec2i& position){ position_ = position; }
	const nytl::Vec2i& position() const { return position_; }

	void commit();
	bool mapped() const { return parent_ && parent_->mapped() && surface_; }

    //res
    virtual unsigned int type() const override { return resourceType::subsurface; }
};

///Represents the subsurface role for a surface resource.
class SubsurfaceRole : public SurfaceRole
{
protected:
	SubsurfaceRes* subsurfaceRes_;

public:
	SubsurfaceRole(SubsurfaceRes& sub) : subsurfaceRes_(&sub) {}

    virtual nytl::Vec2i position() const override { return subsurfaceRes_->position(); }
    virtual bool mapped() const override { return subsurfaceRes_->mapped(); }
    virtual void commit() override { subsurfaceRes_->commit(); }
    virtual unsigned int roleType() const override { return surfaceRoleType::sub; }

	SubsurfaceRes& subsurfaceRes() const { return *subsurfaceRes_; }
};

}
