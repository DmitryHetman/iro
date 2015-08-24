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
    surf->getPending().damage = rect2i(x, y, width, height);
}
void surfaceFrame(wl_client* client, wl_resource* resource, unsigned int callback)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    surf->registerFrameCallback(callback);
}
void surfaceOpaqueRegion(wl_client* client,wl_resource* resource, wl_resource* region)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    regionRes* reg = (regionRes*) wl_resource_get_user_data(region);

    surf->getPending().opaque = reg->getRegion();
}
void surfaceInputRegion(wl_client* client,wl_resource* resource, wl_resource* region)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    regionRes* reg = (regionRes*) wl_resource_get_user_data(region);

    surf->getPending().input = reg->getRegion();
}
void surfaceCommit(wl_client* client, wl_resource* resource)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    surf->commit();
}
void surfaceBufferTransform(wl_client* client, wl_resource* resource, int transform)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    surf->getPending().transform = transform;
}
void surfaceBufferScale(wl_client* client, wl_resource* resource, int scale)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    surf->getPending().scale = scale;
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
    //todo
    mapper_ = iroBackend()->getOutputs()[0];

    pending_ = new surfaceState;
    commited_ = new surfaceState;
}

surfaceRes::~surfaceRes()
{
    unsetRole();
}

void surfaceRes::setShellSurface(unsigned int id)
{
    unsetRole();

    role_ = surfaceRole::shell;
    shellSurface_ = new shellSurfaceRes(*this, getWlClient(), id);
}

void surfaceRes::setSubsurface(unsigned int id, surfaceRes* parent)
{
    unsetRole();

    role_ = surfaceRole::sub;
    subsurface_ = new subsurfaceRes(*this, getWlClient(), id, *parent);

    parent->addChild(*this);
}

void surfaceRes::setCursor(vec2i hotspot)
{
    unsetRole();

    role_ = surfaceRole::cursor;
    cursorHotspot_ = hotspot;
}

void surfaceRes::unsetRole()
{
    if(role_ == surfaceRole::shell)
    {
        shellSurface_->destroy();
    }
    else if(role_ == surfaceRole::sub)
    {
        subsurface_->getParent().removeChild(*this);
        subsurface_->destroy();
    }
    else if(role_ == surfaceRole::cursor && iroPointer()->getCursor() == this)
    {
        iroPointer()->resetCursor();
    }

    role_ = surfaceRole::none;

    if(mapper_)mapper_->unmapSurface(this);
}

void surfaceRes::addChild(surfaceRes& child)
{
    children_.push_back(&child);
}

void surfaceRes::removeChild(surfaceRes& child)
{
    for(unsigned int i(0); i < children_.size(); i++)
    {
        if(children_[i] == &child)
            children_.erase(children_.begin() + i);
    }
}

bool surfaceRes::isChild(surfaceRes* surf) const
{
    for(unsigned int i(0); i < children_.size(); i++)
    {
        if(children_[i] == surf)
            return 1;
    }
    return 0;
}

void surfaceRes::registerFrameCallback(unsigned int id)
{
    callbackRes* cb = new callbackRes(getWlClient(), id);
    callbacks_.push_back(cb);
}

void surfaceRes::frameDone()
{
    for(auto* cb : callbacks_)
    {
        wl_callback_send_done(&cb->getWlResource(), iroTime());
        cb->destroy();
    }

    callbacks_.clear();
}

void surfaceRes::commit()
{
    delete commited_;

    commited_ = pending_;
    pending_ = new surfaceState();

    if(role_ != surfaceRole::none && role_ != surfaceRole::cursor)
    {
        if(commited_->attached.get() && !pending_->attached.get())
        {
            mapper_->mapSurface(this);
        }
        else if(!commited_->attached.get() && pending_->attached.get())
        {
            mapper_->unmapSurface(this);
        }
        else if(commited_->attached.get() && pending_->attached.get())
        {
            mapper_->refresh();
        }
    }

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
}

void surfaceRes::attach(bufferRes& buff, vec2i pos)
{
    iroLog("attached buffer");

    //buff.setReinit();

    pending_->attached.set(&buff);
    pending_->offset = pos;
}

