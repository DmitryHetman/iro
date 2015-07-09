#pragma once

#include <iro.hpp>
#include <resources/resource.hpp>

#include <util/vec.hpp>
#include <util/region.hpp>

#include <wayland-server-core.h>

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
    wl_output_transform transform = wl_output_transform::WL_OUTPUT_TRANSFORM_NORMAL;

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
    };

    std::vector<surfaceRes*> children_;

public:
    surfaceRes(wl_client* client, unsigned int id);
    ~surfaceRes();

    surfaceState& getCommited() { return commited_; }
    const surfaceState& getCommited() const { return commited_; }

    surfaceState& getPending() { return pending_; }
    const surfaceState& getPending() const { return pending_; }

    void commit();

    void setSubsurface(unsigned int id, surfaceRes* parent);
    void setShellSurface(unsigned int id);
    void setCursorRole();
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
