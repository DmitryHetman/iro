#include <iro/compositor/surface.hpp>

#include <iro/compositor/buffer.hpp>
#include <iro/compositor/compositor.hpp>
#include <iro/compositor/region.hpp>
#include <iro/compositor/callback.hpp>
#include <iro/compositor/client.hpp>
#include <iro/backend/output.hpp>
#include <iro/backend/backend.hpp>
#include <iro/backend/surfaceContext.hpp>

#include <ny/base/log.hpp>

#include <wayland-server-protocol.h>

namespace iro
{

class surfaceRendererData{};

//surfaceState
SurfaceState& SurfaceState::operator=(const SurfaceState& other)
{
    opaque = other.opaque;
    input = other.input;
    damage = other.damage;
    offset = other.offset;
    buffer.reset(other.buffer.get());

    frameCallbacks.clear();
    for(auto& ref : other.frameCallbacks)
    {
        if(ref)frameCallbacks.push_back(ref);
    }

    scale = other.scale;
    transform = other.transform;
    zOrder = other.zOrder;

    return *this;
}

//surface wayland
void surfaceDestroy(wl_client*, wl_resource* resource)
{
	SurfaceRes* surf = Resource::validateDisconnect<SurfaceRes>(resource, "surfaceDestroy");
	if(!surf) return;

    surf->destroy();
}
void surfaceAttach(wl_client*, wl_resource* resource, wl_resource* wlbuffer, int x, int y)
{
    SurfaceRes* surf = Resource::validateDisconnect<SurfaceRes>(resource, "surfaceAttach");
	if(!surf) return;

	BufferRes* buff = Resource::validate<BufferRes>(*wlbuffer);
	if(!buff)
	{
		buff = &surf->client().addResource(std::make_unique<BufferRes>(*wlbuffer));
	}

    surf->attach(*buff, {x,y});
}
void surfaceDamage(wl_client*, wl_resource* resource, int x, int y, int width, int height)
{
	SurfaceRes* surf = Resource::validateDisconnect<SurfaceRes>(resource, "surfaceDamage");
	if(!surf) return;

    surf->damage({x, y, width, height});
}
void surfaceFrame(wl_client* client, wl_resource* resource, unsigned int id)
{
	SurfaceRes* surf = Resource::validateDisconnect<SurfaceRes>(resource, "surfaceFrame");
	if(!surf) return;

	auto cb = std::make_unique<CallbackRes>(*client, id);
    surf->addFrameCallback(*cb);

	surf->client().addResource(std::move(cb));
}
void surfaceOpaqueRegion(wl_client*,wl_resource* resource, wl_resource* region)
{
	SurfaceRes* surf = Resource::validateDisconnect<SurfaceRes>(resource, "surfaceOpaqueRegion");
	RegionRes* reg = Resource::validateDisconnect<RegionRes>(region, "surfaceOpaqueRegion");
	if(!surf || !reg) return;

    surf->opaqueRegion(reg->region());
}
void surfaceInputRegion(wl_client*,wl_resource* resource, wl_resource* region)
{
	SurfaceRes* surf = Resource::validateDisconnect<SurfaceRes>(resource, "surfaceOpaqueRegion");
	RegionRes* reg = Resource::validateDisconnect<RegionRes>(region, "surfaceOpaqueRegion");
	if(!surf || !reg) return;

    surf->inputRegion(reg->region());
}
void surfaceCommit(wl_client*, wl_resource* resource)
{
	SurfaceRes* surf = Resource::validateDisconnect<SurfaceRes>(resource, "surfaceCommit");
	if(!surf) return;

    surf->commit();
}
void surfaceBufferTransform(wl_client*, wl_resource* resource, int transform)
{
	SurfaceRes* surf = Resource::validateDisconnect<SurfaceRes>(resource, "surfaceBufferTransform");
	if(!surf) return;

    surf->bufferTransform(transform);
}
void surfaceBufferScale(wl_client*, wl_resource* resource, int scale)
{
	SurfaceRes* surf = Resource::validateDisconnect<SurfaceRes>(resource, "surfaceBufferScale");
	if(!surf) return;

    surf->bufferScale(scale);
}

const struct wl_surface_interface surfaceImplementation =
{
    &surfaceDestroy,
    &surfaceAttach,
    &surfaceDamage,
    &surfaceFrame,
    &surfaceOpaqueRegion,
    &surfaceInputRegion,
    &surfaceCommit,
    &surfaceBufferTransform,
    &surfaceBufferScale
};


//surface implementation
SurfaceRes::SurfaceRes(wl_client& client, unsigned int id) 
	: Resource(client, id, &wl_surface_interface, &surfaceImplementation, 3),
	surfaceContext_(nullptr)
{
	if(!compositor().backend())
	{
		ny::sendWarning("SurfaceRes::SurfaceRes: no valid backend, cant create context.");
		return;
	}

	surfaceContext_ = compositor().backend()->createSurfaceContext();
	if(!surfaceContext_)
	{
		ny::sendWarning("SurfaceRes::SurfaceRes: failed to create context.");
	}
}

SurfaceRes::~SurfaceRes()
{
	for(auto* outp : mappedOutputs_)
		outp->unmapSurface(*this);
}

nytl::Vec2i SurfaceRes::position() const
{
    if(role_)
        return role_->position() + commited_.offset;

    return commited_.offset;
}

nytl::Vec2ui SurfaceRes::size() const
{
    if(commited_.buffer)
        return bufferSize_ * commited_.scale;

    return {0, 0};
}

nytl::Rect2i SurfaceRes::extents() const
{
    return {position(), size()};
}

void SurfaceRes::sendFrameDone()
{
    for(auto& cb : commited_.frameCallbacks)
    {
        if(cb)
        {
            wl_callback_send_done(&cb->wlResource(), compositor().time());
            cb->destroy();
        }
    }

    commited_.frameCallbacks.clear();
}

void SurfaceRes::addFrameCallback(CallbackRes& cb)
{
    pending_.frameCallbacks.push_back(&cb);
}

void SurfaceRes::bufferScale(int scale)
{
    pending_.scale = scale;
}

void SurfaceRes::bufferTransform(unsigned int transform)
{
    pending_.transform = transform;
}

void SurfaceRes::inputRegion(const nytl::RectRegion2i& input)
{
    pending_.input = input;
}

void SurfaceRes::opaqueRegion(const nytl::RectRegion2i& opaque)
{
    pending_.opaque = opaque;
}

void SurfaceRes::damage(const nytl::Rect2i& dmg)
{
    pending_.damage.add(dmg);
}

void SurfaceRes::attach(BufferRes& buff, const nytl::Vec2i& pos)
{
    pending_.offset = pos;
    pending_.buffer.reset(buff);
}

void SurfaceRes::commit()
{
	ny::sendLog("commiting surfaceRes ", this, " with buffer ", pending_.buffer.get());

    commited_ = pending_;
    pending_.reset();

    if(role_)
    {
        role_->commit();
    }

    for(auto* o : mappedOutputs_)
    {
        o->unmapSurface(*this);
    }
    mappedOutputs_.clear();

    if(commited_.buffer.get())
    {
        if(!surfaceContext_)
		{
			ny::sendWarning("SurfaceRes::commit: no valiud surfaceContext_");
			return;
		}

		if(!surfaceContext_->attachBuffer(*commited_.buffer.get(), bufferSize_))
		{
			ny::sendWarning("SurfaceRes::commit: attaching the buffer failed");
			return;
		}

		commited_.buffer->sendRelease();
		remap();
    }
}

void SurfaceRes::remap()
{
	if(!mapped()) return;

	auto* bckn = compositor().backend();
	if(!bckn)
	{
		ny::sendWarning("SurfaceRes::commit: compositor has no valid backend");
		return;
	}

	mappedOutputs_ = bckn->outputsAt(extents());
	ny::sendLog("found mapOutputs: ", mappedOutputs_.size());
	for(auto* o : mappedOutputs_)
	{
		o->mapSurface(*this);
	}
}

SurfaceRole& SurfaceRes::role(std::unique_ptr<SurfaceRole> role)
{
	if(!role)
	{
		ny::sendWarning("SurfaceRes::role: nullptr as parameter not allowed");
		return *role; //aargh
	}

	if(roleType() == surfaceRoleType::none)
	{
		roleType_ = role->roleType();
	}
	else if(roleType() != role->roleType())
	{
		ny::sendWarning("SurfaceRes::role: surface already has a different role");
		return *role_;
	}

	role_ = std::move(role);
	return *role_;
}

void SurfaceRes::clearRole()
{
	if(!role_)
	{
		ny::sendWarning("SurfacEres::clearRole: surface has no role");
		return;
	}

	role_.reset();
}

unsigned int SurfaceRes::roleType() const
{
	return roleType_;
}

bool SurfaceRes::mapped() const
{
    return (commited_.buffer && role_ && role_->mapped());
}

}
