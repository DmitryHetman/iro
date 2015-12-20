#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>
#include <iro/compositor/buffer.hpp>
#include <iro/compositor/callback.hpp>

#include <nytl/vec.hpp>
#include <nytl/region.hpp>
#include <nytl/rect.hpp>
#include <nytl/watchable.hpp>

#include <vector>
#include <memory>

namespace iro
{

///The surfaceState class represents the attributes a surface can have
///that are changed with a surface::commit().
class SurfaceState
{
public:
	nytl::region2i opaque = nytl::region2i();
	nytl::region2i input = nytl::region2i();
	nytl::region2i damage = nytl::region2i();
	nytl::vec2i offset = nytl::vec2i();

    BufferRef buffer;
    std::vector<CallbackRes*> frameCallbacks;

    int scale = 1;
    unsigned int transform = 0;
    int zOrder = 0;

    void reset()
    {
        opaque = nytl::region2i();
        input = nytl::region2i();
        damage = nytl::region2i();
        offset = nytl::vec2i();
        buffer.set(nullptr);
        frameCallbacks.clear(); //todo ?
        scale = 1;
        transform = 0;
        zOrder = 0;
    }

    SurfaceState& operator=(const SurfaceState& other);
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
}

///Base class for a surface role, such as subSurface or shellSurface
class SurfaceRole
{
public:
    virtual unsigned roleType() const = 0;
    virtual nytl::vec2i position() const = 0;
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
	nytl::vec2ui bufferSize_;

    ///Backend and renderer specific context.
	std::unique_ptr<SurfaceContext> surfaceContext_;
	std::unique_ptr<SurfaceRole> role_;

public:
    SurfaceRes(wl_client& client, unsigned int id);
    ~SurfaceRes();

	//those funtions affect pending state
    void addFrameCallback(CallbackRes& cb);
    void inputRegion(const nytl::region2i& input);
	void opaqueRegion(const nytl::region2i& output);
 	void bufferScale(int scale);
	void bufferTransform(unsigned int transform);
    void attach(BufferRes& buff, const nytl::vec2i& pos);
	void damage(const nytl::rect2i& dmg);

	///Commits the pending surface state and makes it the current one.
    void commit();

	//those functions inform about the current surface state
    const nytl::region2i& inputRegion() const { return commited_.input; }
    const nytl::region2i& opaqueRegion() const { return commited_.opaque; }
    const nytl::region2i& damageRegion() const { return commited_.damage; }
    int bufferScale() const { return commited_.scale; }
    unsigned int bufferTransform() const { return commited_.transform; }
    BufferRes* attachedBuffer() const { return commited_.buffer.get(); }

    bool isMapped() const;
    void sendFrameDone();

    SurfaceRole* role() const { return role_.get(); }
	SurfaceRole& role(std::unique_ptr<SurfaceRole>&& role);
    unsigned int roleType() const;

	nytl::vec2i position() const;
	nytl::vec2ui size() const;
	nytl::rect2i extents() const;

	SurfaceContext* surfaceContext() const { return surfaceContext_.get(); }

    //resource
    virtual unsigned int type() const override { return resourceType::surface; }
};

}
