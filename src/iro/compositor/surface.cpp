#include <iro/compositor/surface.hpp>

#include <iro/compositor/shellSurface.hpp>
#include <iro/compositor/subsurface.hpp>
#include <iro/compositor/buffer.hpp>
#include <iro/compositor/region.hpp>
#include <iro/compositor/callback.hpp>
#include <iro/backend/renderer.hpp>
#include <iro/backend/output.hpp>
#include <iro/backend/backend.hpp>
#include <iro/seat/seat.hpp>
#include <iro/seat/pointer.hpp>
#include <iro/util/log.hpp>

#include <wayland-server-protocol.h>

surfaceState& surfaceState::operator=(const surfaceState& other)
{
    opaque = other.opaque;
    input = other.input;
    damage = other.damage;
    offset = other.offset;
    buffer.set(other.buffer.get());

    frameCallbacks.clear();
    for(auto& ref : other.frameCallbacks)
    {
        if(ref.get())frameCallbacks.push_back(callbackRef(*ref.get()));
    }

    scale = other.scale;
    transform = other.transform;
    zOrder = other.zOrder;

    return *this;
}
/////
void surfaceDestroy(wl_client* client, wl_resource* resource)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    surf->destroy();
}
void surfaceAttach(wl_client* client, wl_resource* resource, wl_resource* wlbuffer, int x, int y)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);

    bufferRes& buff = bufferForResource(*wlbuffer); //gets or created bufferRes for this buffer wl_resource
    surf->attach(buff, vec2i(x,y));
}
void surfaceDamage(wl_client* client, wl_resource* resource, int x, int y, int width, int height)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    surf->damage(rect2i(x, y, width, height));
}
void surfaceFrame(wl_client* client, wl_resource* resource, unsigned int id)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    callbackRes* cb = new callbackRes(*client, id);

    surf->addFrameCallback(*cb);
}
void surfaceOpaqueRegion(wl_client* client,wl_resource* resource, wl_resource* region)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    regionRes* reg = (regionRes*) wl_resource_get_user_data(region);

    surf->setOpaqueRegion(reg->getRegion());
}
void surfaceInputRegion(wl_client* client,wl_resource* resource, wl_resource* region)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    regionRes* reg = (regionRes*) wl_resource_get_user_data(region);

    surf->setInputRegion(reg->getRegion());
}
void surfaceCommit(wl_client* client, wl_resource* resource)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    surf->commit();
}
void surfaceBufferTransform(wl_client* client, wl_resource* resource, int transform)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    surf->setBufferTransform(transform);
}
void surfaceBufferScale(wl_client* client, wl_resource* resource, int scale)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    surf->setBufferScale(scale);
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


//////////////////////////////////////////////////////////////////////////////
surfaceRes::surfaceRes(wl_client& client, unsigned int id) : resource(client, id, &wl_surface_interface, &surfaceImplementation, 3)
{
}

surfaceRes::~surfaceRes()
{
    if(renderData_)
        delete renderData_;
}

vec2i surfaceRes::getPosition() const
{
    if(role_)
        return role_->getPosition() + pending_.offset;

    return pending_.offset;
}

vec2ui surfaceRes::getSize() const
{
    if(commited_.buffer.get())
        return commited_.buffer.get()->getSize() * commited_.scale;

    return vec2ui();
}

rect2i surfaceRes::getExtents() const
{
    return rect2i(getPosition(), getSize());
}

void surfaceRes::frameDone()
{
    for(auto& cb : commited_.frameCallbacks)
    {
        if(cb.get())
        {
            wl_callback_send_done(&cb.get()->getWlResource(), iroTime());
            cb.get()->destroy();
        }
    }

    commited_.frameCallbacks.clear();
}

void surfaceRes::addFrameCallback(callbackRes& cb)
{
    pending_.frameCallbacks.push_back(callbackRef(cb));
}

void surfaceRes::setBufferScale(int scale)
{
    pending_.scale = scale;
}

void surfaceRes::setBufferTransform(unsigned int transform)
{
    pending_.transform = transform;
}

void surfaceRes::setInputRegion(region input)
{
    pending_.input = input;
}

void surfaceRes::setOpaqueRegion(region opaque)
{
    pending_.opaque = opaque;
}

void surfaceRes::damage(rect2i dmg)
{
    pending_.damage.add(dmg);
}

void surfaceRes::attach(bufferRes& buff, vec2i pos)
{
    pending_.offset = pos;
    pending_.buffer.set(&buff);
}

void surfaceRes::commit()
{
    //todo

    commited_ = pending_;
    pending_.reset();

    if(role_)
    {
        role_->commit();
    }

    if(commited_.buffer.get())
    {
        for(auto* o : outputs_)
        {
            o->unmapSurface(*this);
        }

        iroRenderer()->attachSurface(*this, *commited_.buffer.get());
        outputs_ = outputsAt(getExtents());

        for(auto* o : outputs_)
        {
            o->mapSurface(*this);
        }
    }
    else
    {
        for(auto* o : outputs_)
        {
            o->unmapSurface(*this);
        }

        outputs_.clear();
    }
}

bool surfaceRes::isMapped() const
{
    return (commited_.buffer.get() && role_ && role_->isMapped());
}
