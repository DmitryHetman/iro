#include <resources/surface.hpp>

#include <resources/shellSurface.hpp>
#include <resources/subsurface.hpp>
#include <resources/buffer.hpp>
#include <backend/output.hpp>
#include <backend/backend.hpp>
#include <seat/seat.hpp>
#include <seat/pointer.hpp>

void surfaceDestroy(wl_client* client, wl_resource* resource)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    delete surf;
}
void surfaceAttach(wl_client* client, wl_resource* resource, wl_resource* wlbuffer, int x, int y)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    surf->getPending().attached = new bufferRes(wlbuffer);
    surf->getPending().offset = vec2i(x, y);
}
void surfaceDamage(wl_client* client, wl_resource* resource, int x, int y, int width, int height)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    surf->getPending().damage = rect2i(x, y, width, height);
}
void surfaceFrame(wl_client* client, wl_resource* resource, unsigned int callback)
{
}
void surfaceOpaqueRegion(wl_client* client,wl_resource* resource, wl_resource* region)
{
}
void surfaceInputRegion(wl_client* client,wl_resource* resource, wl_resource* region)
{
}
void surfaceCommit(wl_client* client, wl_resource* resource)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(resource);
    surf->commit();
}
void surfaceBufferTransform(wl_client* client, wl_resource* resource, int transform)
{
}
void surfaceBufferScale(wl_client* client, wl_resource* resource, int scale)
{
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

void destroySurface(wl_resource* res)
{
    surfaceRes* surf = (surfaceRes*) wl_resource_get_user_data(res);
    delete surf;
}

//////////////////////////////////////////////////////////////////////////////
surfaceRes::surfaceRes(wl_client* client, unsigned int id) : resource(client, id, &wl_surface_interface, &surfaceImplementation, 3, this, destroySurface)
{
}

surfaceRes::~surfaceRes()
{
    unsetRole();
}

void surfaceRes::setShellSurface(unsigned int id)
{
    unsetRole();

    role_ = surfaceRole::shell;
    shellSurface_ = new shellSurfaceRes(this, getWlClient(), id);
}

void surfaceRes::setSubsurface(unsigned int id, surfaceRes* parent)
{
    unsetRole();

    role_ = surfaceRole::sub;
    subsurface_ = new subsurfaceRes(this, getWlClient(), id, parent);

    parent->addChild(this);
}

void surfaceRes::setCursorRole(vec2ui hotspot)
{
    unsetRole();

    role_ = surfaceRole::cursor;
    cursorHotspot_ = hotspot;
}

void surfaceRes::unsetRole()
{
    if(role_ == surfaceRole::shell)
    {
        delete shellSurface_;
    }
    else if(role_ == surfaceRole::sub)
    {
        subsurface_->getParent()->removeChild(this);
        delete subsurface_;
    }
    else if(role_ == surfaceRole::cursor && getSeat()->getPointer()->getCursor() == this)
    {
        getSeat()->getPointer()->setCursor(nullptr, vec2ui(0, 0));
    }

    role_ = surfaceRole::none;
    getBackend()->getOutput()->unmapSurface(this);
}

void surfaceRes::addChild(surfaceRes* child)
{
    children_.push_back(child);
}

void surfaceRes::removeChild(surfaceRes* child)
{
    for(unsigned int i(0); i < children_.size(); i++)
    {
        if(children_[i] == child)
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

void surfaceRes::commit()
{
    surfaceState old = commited_;

    commited_ = pending_;
    pending_ = surfaceState();

    if(role_ != surfaceRole::none && role_ != surfaceRole::cursor)
    {
        if(commited_.attached && !old.attached)
        {
            getBackend()->getOutput()->mapSurface(this);
        }
        else if(!commited_.attached && old.attached)
        {
            getBackend()->getOutput()->unmapSurface(this);
        }
        else if(commited_.attached && old.attached)
        {
            getBackend()->getOutput()->refresh();
        }
    }

    if(old.attached)
        old.attached->destroy();
}



