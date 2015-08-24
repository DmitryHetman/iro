#include <iro/compositor/surface.hpp>

#include <iro/compositor/shellSurface.hpp>
#include <iro/compositor/subsurface.hpp>
#include <iro/compositor/buffer.hpp>
#include <iro/compositor/region.hpp>
#include <iro/compositor/callback.hpp>
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
        frameCallbacks.push_back(callbackRef(ref.get()));
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

    bufferRes* buff = bufferForResource(*wlbuffer); //gets or created bufferRes for this buffer wl_resource
    surf->attach(*buff, vec2i(x,y));
}
void surfaceDamage(wl_client* client, wl_resource* resource, int x, int y, int width, int height)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    surf->damage(rect2i(x, y, width, height);
}
void surfaceFrame(wl_client* client, wl_resource* resource, unsigned int callback)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    surf->addFrameCallback(callback);
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
<<<<<<< HEAD
    //todo
    mapper_ = iroBackend()->getOutputs()[0];

    pending_ = new surfaceState;
    commited_ = new surfaceState;
=======
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527
}

surfaceRes::~surfaceRes()
{
<<<<<<< HEAD
    unsetRole();
=======
    if(renderData_)
        delete renderData_;
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527
}

vec2i surfaceRes::getPosition()
{
    if(role_)
        return role_->getPosition() + pending_.offset;

    return pending_.offset;
}

vec2ui surfaceRes::getSize()
{
    if(commited_.attached.get())
        return commited_.attached.get()->getSize() * commited_.scale;
}

rect2i surfaceRes::getExtents()
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
<<<<<<< HEAD
    delete commited_;

    commited_ = pending_;
    pending_ = new surfaceState();
=======
    //todo

    commited_ = pending_;
    pending_.reset();
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527

    if(role_)
    {
<<<<<<< HEAD
        if(commited_->attached.get() && !pending_->attached.get())
        {
            mapper_->mapSurface(this);
        }
        else if(!commited_->attached.get() && pending_->attached.get())
=======
        role_->commit();
    }

    if(commited_.bufer.get())
    {
        for(auto* o : outputs_)
        {
            o->unmapSurface(*this);
        }

        getIro()->getRenderer()->attachSurface(*this, *commited_.buffer.get());
        outputs_ = outputsIntersecting(getExtents());

        for(auto* o : outputs_)
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527
        {
            o->mapSurface(*this);
        }
<<<<<<< HEAD
        else if(commited_->attached.get() && pending_->attached.get())
=======
    }
    else
    {
        for(auto o* : outputs_)
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527
        {
            o->unmapSurface(*this);
        }

<<<<<<< HEAD
}

vec2i surfaceRes::getPosition() const
{
    vec2i ret;

    if(role_ == surfaceRole::shell)
        ret += shellSurface_->getToplevelPosition();

    return ret;
}

rect2i surfaceRes::getExtents() const
{
    return rect2i(getPosition(), (commited_->attached.get()) ? commited_->attached.get()->getSize() * commited_->scale : vec2ui(0,0));
=======
        outputs_.clear();
    }
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527
}

bool surfaceRes::isMapped()
{
<<<<<<< HEAD
    iroLog("attached buffer");

    //buff.setReinit();

    pending_->attached.set(&buff);
    pending_->offset = pos;
=======
    return (commited_.buffer.get && role_ && role_->isMapped());
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527
}
