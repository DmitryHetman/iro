#pragma once

#include <iro.hpp>
#include <resources/resource.hpp>

#include <util/vec.hpp>

//user data of subsurface is the corresponding surface. Use then getSubsurfac() to get a subsurfaceRes object
class subsurfaceRes : public resource
{
protected:
    friend surfaceRes;
    subsurfaceRes(surfaceRes* surface, wl_client* client, unsigned int id, surfaceRes* parent);

    surfaceRes* surface_ = nullptr;
    surfaceRes* parent_ = nullptr;
    bool sync_ = 0;

    vec2i position_;

public:
    surfaceRes* getSurface() const { return surface_; };

    surfaceRes* getParent() const { return parent_; }
    bool isSync() const { return sync_; }

    void setSync(bool sync){ sync_ = sync; }
    void setPosition(vec2i position){ position_ = position; }

    resourceType getType() const { return resourceType::subsurface; }
};
