#pragma once

#include <iro.hpp>
#include <resources/resource.hpp>

#include <util/vec.hpp>
#include <util/region.hpp>

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

    union
    {
        shellSurfaceRes* shellSurface_;
        subsurfaceRes* subsurface_;
        vec2ui cursorHotspot_;
    };

    std::vector<surfaceRes*> children_;
    callbackRes* callback_ = nullptr;

public:
    surfaceRes(wl_client* client, unsigned int id);
    ~surfaceRes();

    surfaceState& getCommited() { return commited_; }
    const surfaceState& getCommited() const { return commited_; }

    surfaceState& getPending() { return pending_; }
    const surfaceState& getPending() const { return pending_; }

    void registerFrameCallback(unsigned int id);

    void commit();

    void setSubsurface(unsigned int id, surfaceRes* parent);
    void setShellSurface(unsigned int id);
    void setCursorRole(vec2ui hotspot);
    void unsetRole();

    void addChild(surfaceRes* child);
    void removeChild(surfaceRes* child);

    surfaceRole getRole() const { return role_; }

    shellSurfaceRes* getShellSurface() const { if(role_ == surfaceRole::shell) return shellSurface_; return nullptr; }
    subsurfaceRes* getSubsurface() const { if(role_ == surfaceRole::sub) return subsurface_; return nullptr; }

    const std::vector<surfaceRes*>& getChildren() const { return children_; }
    bool isChild(surfaceRes* surf) const;

    resourceType getType() const { return resourceType::surface; }
};
