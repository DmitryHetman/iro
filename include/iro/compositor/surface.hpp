#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>
#include <iro/compositor/buffer.hpp>
#include <iro/compositor/callback.hpp>

#include <nytl/vec.hpp>
#include <nytl/rectRegion.hpp>
#include <nytl/rect.hpp>
#include <nytl/observe.hpp>

#include <vector>
#include <memory>

namespace iro
{

///The surfaceState class represents the attributes a surface can have
///that are changed with a surface::commit().
class SurfaceState
{
public:
    void reset()
    {
        opaque = {}; 
        input = {};
        damage = {};
        offset = {};
        buffer.reset(nullptr);
        frameCallbacks.clear(); //todo ?
        scale = 1;
        transform = 0;
        zOrder = 0;
    }

    SurfaceState& operator=(const SurfaceState& other);

public:
	nytl::RectRegion2i opaque = {};
	nytl::RectRegion2i input = {};
	nytl::RectRegion2i damage = {};
	nytl::Vec2i offset = {};

    BufferPtr buffer;
    std::vector<CallbackRes*> frameCallbacks;

    int scale = 1;
    unsigned int transform = 0;
    int zOrder = 0;
};


///All possible roleType ids for a surface should be added to this namespace as
///constexpr unsigned int.
namespace surfaceRoleType
{
    constexpr unsigned int none = 0;
    constexpr unsigned int shell = 1;
    constexpr unsigned int sub = 2;
    constexpr unsigned int cursor = 3;
    constexpr unsigned int dataSource = 4;
	constexpr unsigned int xdgSurface = 5;
	constexpr unsigned int xdgPopup = 6;
}

///Base class for a surface role, such as subSurface or shellSurface.
///Surface rols are expected to call clearRole() on their SurfaceRes when they get
///destructed.
class SurfaceRole
{
public:
    virtual unsigned roleType() const = 0;
    virtual nytl::Vec2i position() const = 0;
    virtual bool mapped() const = 0;
    virtual void commit() = 0;
};

///The surfaceRes class represents a client surface. 
///It holds a current surfaceState and a pending surfaceState with all wayland
///surface attributes, as well as keeps track of all outputs the surface is mapped on and
///the role the surface has. Additionally the renderer can store its renderData inside the
///surface.
class SurfaceRes : public Resource
{
protected:
    SurfaceState commited_;
    SurfaceState pending_;

	///All outputs this surface is currently mapped on.
    std::vector<Output*> mappedOutputs_; 
	nytl::Vec2ui bufferSize_;

    ///Backend and renderer specific context.
	std::unique_ptr<SurfaceContext> surfaceContext_;

	///The roleType of the surface. Once set (with a value != none) it cant be changed anymore.
	unsigned int roleType_ = surfaceRoleType::none; 
	std::unique_ptr<SurfaceRole> role_;

public:
    SurfaceRes(wl_client& client, unsigned int id);
    ~SurfaceRes();

	//those funtions affect pending state
    void addFrameCallback(CallbackRes& cb);
    void inputRegion(const nytl::RectRegion2i& input);
	void opaqueRegion(const nytl::RectRegion2i& output);
 	void bufferScale(int scale);
	void bufferTransform(unsigned int transform);
    void attach(BufferRes& buff, const nytl::Vec2i& pos);
	void damage(const nytl::Rect2i& dmg);

	///Commits the pending surface state and makes it the current one.
    void commit();

	///Updates the surface mappings
	void remap();

	//those functions inform about the current surface state
    const nytl::RectRegion2i& inputRegion() const { return commited_.input; }
    const nytl::RectRegion2i& opaqueRegion() const { return commited_.opaque; }
    const nytl::RectRegion2i& damageRegion() const { return commited_.damage; }
    int bufferScale() const { return commited_.scale; }
    unsigned int bufferTransform() const { return commited_.transform; }
    BufferRes* attachedBuffer() const { return commited_.buffer.get(); }

    bool mapped() const;
    void sendFrameDone();

    SurfaceRole* role() const { return role_.get(); }
	SurfaceRole& role(std::unique_ptr<SurfaceRole> role);
	void clearRole();
    unsigned int roleType() const;

	nytl::Vec2i position() const;
	nytl::Vec2ui size() const;
	nytl::Rect2i extents() const;

	SurfaceContext& surfaceContext() const { return *surfaceContext_; }

    //resource
    virtual unsigned int type() const override { return resourceType::surface; }
};

}
