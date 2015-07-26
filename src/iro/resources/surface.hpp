#pragma once

#include <iro/include.hpp>
#include <iro/resources/resource.hpp>

#include <nyutil/vec.hpp>
#include <nyutil/region.hpp>

#include <vector>

//state////////////////////////////////////////////////////
struct surfaceState
{
    region opaque = region();
    region input = region();

    rect2i damage = rect2i(0, 0, 0, 0);
    vec2i offset = vec2i(0, 0);
    bufferRes* attached = nullptr;

    int scale = 1;
    unsigned int transform = 0;

    int zOrder = 0;
};

enum class surfaceRole : unsigned char
{
    none = 0,

    shell,
    sub,
    cursor
};

//class///////////////////////////////////////////////////
class surfaceRes : public resource
{
protected:
    surfaceState commited_;
    surfaceState pending_;

    surfaceRole role_ = surfaceRole::none;

    output* mapper_ = nullptr;

    union
    {
        shellSurfaceRes* shellSurface_;
        subsurfaceRes* subsurface_;
        vec2i cursorHotspot_;
    };

    std::vector<surfaceRes*> children_;
    callbackRes* callback_ = nullptr;

public:
    surfaceRes(wl_client& client, unsigned int id);
    ~surfaceRes();

    const surfaceState& getCommited() const { return commited_; }
    surfaceState& getPending() { return pending_; }
    const surfaceState& getPending() const { return pending_; }

    void registerFrameCallback(unsigned int id);
    void frameDone();

    void commit();

    void setSubsurface(unsigned int id, surfaceRes* parent);
    void setShellSurface(unsigned int id);
    void setCursor(vec2i hotspot);
    void unsetRole();

    void addChild(surfaceRes& child);
    void removeChild(surfaceRes& child);

    surfaceRole getRole() const { return role_; }

    shellSurfaceRes* getShellSurface() const { if(role_ == surfaceRole::shell) return shellSurface_; return nullptr; }
    subsurfaceRes* getSubsurface() const { if(role_ == surfaceRole::sub) return subsurface_; return nullptr; }

    vec2i getPosition() const;
    rect2i getExtents() const;

    const std::vector<surfaceRes*>& getChildren() const { return children_; }
    bool isChild(surfaceRes* surf) const;

    //resource
    resourceType getType() const { return resourceType::surface; }
};
