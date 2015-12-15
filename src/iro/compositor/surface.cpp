#include <iro/compositor/surface.hpp>

#include <iro/compositor/buffer.hpp>
#include <iro/compositor/compositor.hpp>
#include <iro/compositor/region.hpp>
#include <iro/compositor/callback.hpp>
#include <iro/compositor/client.hpp>
#include <iro/backend/output.hpp>
#include <iro/backend/backend.hpp>
#include <iro/backend/surfaceContext.hpp>

#include <nytl/log.hpp>
#include <nytl/make_unique.hpp>

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
    buffer.set(other.buffer.get());

    frameCallbacks.clear();
    for(auto& ref : other.frameCallbacks)
    {
        if(ref.get())frameCallbacks.push_back(CallbackRef(*ref.get()));
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
		buff = &surf->client().addResource(nytl::make_unique<BufferRes>(*wlbuffer));
	}

    surf->attach(*buff, nytl::vec2i(x,y));
}
void surfaceDamage(wl_client*, wl_resource* resource, int x, int y, int width, int height)
{
	SurfaceRes* surf = Resource::validateDisconnect<SurfaceRes>(resource, "surfaceDamage");
	if(!surf) return;

    surf->damage(nytl::rect2i(x, y, width, height));
}
void surfaceFrame(wl_client* client, wl_resource* resource, unsigned int id)
{
	SurfaceRes* surf = Resource::validateDisconnect<SurfaceRes>(resource, "surfaceFrame");
	if(!surf) return;

	auto cb = nytl::make_unique<CallbackRes>(*client, id);
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
		nytl::sendWarning("SurfaceRes::SurfaceRes: no valid backend, cant create context.");
		return;
	}

	surfaceContext_ = compositor().backend()->createSurfaceContext();
	if(!surfaceContext_)
	{
		nytl::sendWarning("SurfaceRes::SurfaceRes: failed to create context.");
	}
}

SurfaceRes::~SurfaceRes()
{
	for(auto* outp : mappedOutputs_)
		outp->unmapSurface(*this);
}

nytl::vec2i SurfaceRes::position() const
{
    if(role_)
        return role_->position() + pending_.offset;

    return pending_.offset;
}

nytl::vec2ui SurfaceRes::size() const
{
    if(commited_.buffer)
        return bufferSize_ * commited_.scale;

    return {0, 0};
}

nytl::rect2i SurfaceRes::extents() const
{
    return nytl::rect2i(position(), size());
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
    pending_.frameCallbacks.push_back(CallbackRef(cb));
}

void SurfaceRes::bufferScale(int scale)
{
    pending_.scale = scale;
}

void SurfaceRes::bufferTransform(unsigned int transform)
{
    pending_.transform = transform;
}

void SurfaceRes::inputRegion(const nytl::region2i& input)
{
    pending_.input = input;
}

void SurfaceRes::opaqueRegion(const nytl::region2i& opaque)
{
    pending_.opaque = opaque;
}

void SurfaceRes::damage(const nytl::rect2i& dmg)
{
    pending_.damage.add(dmg);
}

void SurfaceRes::attach(BufferRes& buff, const nytl::vec2i& pos)
{
    pending_.offset = pos;
    pending_.buffer.set(buff);
}

void SurfaceRes::commit()
{
	nytl::sendLog("commiting surfaceRes ", this, " with buffer ", pending_.buffer.get());

    commited_ = pending_;
    pending_.reset();

    if(role_)
    {
        role_->commit();
    }

    if(commited_.buffer.get())
    {
        for(auto* o : mappedOutputs_)
        {
            o->unmapSurface(*this);
        }

        if(surfaceContext_)
		{
			if(!surfaceContext_->attachBuffer(*commited_.buffer.get(), bufferSize_))
			{
				nytl::sendWarning("SurfaceRes::commit: attaching the buffer failed");
				return;
			}
		}
		else
		{
			nytl::sendWarning("SurfaceRes::commit: no valiud surfaceContext_");
			return;
		}

		auto* bckn = compositor().backend();
		if(!bckn)
		{
			nytl::sendWarning("SurfaceRes::commit: compositor has no valid backend");
			return;
		}

        mappedOutputs_ = bckn->outputsAt(extents());
		nytl::sendLog("mapping surfaceRes ", this, " on ", mappedOutputs_.size(), " outputs");
        for(auto* o : mappedOutputs_)
        {
            o->mapSurface(*this);
        }
    }
    else
    {
        for(auto* o : mappedOutputs_)
        {
            o->unmapSurface(*this);
        }

        mappedOutputs_.clear();
    }

}

SurfaceRole& SurfaceRes::role(std::unique_ptr<SurfaceRole>&& role)
{
	role_ = std::move(role);
	return *role_;
}

unsigned int SurfaceRes::roleType() const
{
	if(role_) return role_->roleType();
	return surfaceRoleType::none;
}

bool SurfaceRes::isMapped() const
{
    return (commited_.buffer && role_ && role_->mapped());
}

}
