#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>
#include <iro/compositor/surface.hpp>

#include <nyutil/vec.hpp>

class subsurfaceRes : public resource, public surfaceRole
{
protected:
    surfaceRes& surface_;
    surfaceRes& parent_;
    bool sync_ = 0;

    vec2i position_;

public:
    subsurfaceRes(surfaceRes& surface, wl_client& client, unsigned int id, surfaceRes& parent);
    surfaceRes& getSurface() const { return surface_; };

    surfaceRes& getParent() const { return parent_; }
    bool isSync() const { return sync_; }

    void setSync(bool sync){ sync_ = sync; }
    void setPosition(vec2i position){ position_ = position; }

    //surfaceRole
    virtual vec2i getPosition() const override { return position_; }
    virtual bool isMapped() const override { return 1; } //todo?
    virtual void commit() override { } //todo
    virtual unsigned char getRoleType() const override { return surfaceRoleType::sub; }

    //res
    virtual resourceType getType() const override { return resourceType::subsurface; }
};
