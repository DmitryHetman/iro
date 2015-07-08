#pragma once

#include <iro.hpp>
#include <resources/resource.hpp>

#include <util/vec.hpp>

#include <wayland-server-core.h>

//user data of subsurface is the corresponding surface. Use then getSubsurfac() to get a subsurfaceRes object
class subsurfaceRes : public resource
{
protected:
    friend surfaceRes;

    subsurfaceRes(surfaceRes* surface, wl_client* client, unsigned int id, surfaceRes* parent);

    surfaceRes* parent_ = nullptr;
    bool sync_ = 0;

    vec2ui position_;

public:
    surfaceRes* getParent() const { return parent_; }
    bool isSync() const { return sync_; }

    void setSync(bool sync){ sync_ = sync; }
    void setPosition(vec2ui position){ position_ = position; }

    resourceType getType() const { return resourceType::subsurface; }
};
