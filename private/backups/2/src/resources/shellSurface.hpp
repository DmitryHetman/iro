#pragma once

#include <iro.hpp>
#include <resources/resource.hpp>
#include <util/vec.hpp>

#include <wayland-server-core.h>

#include <string>

//todo: state
enum class shellSurfaceState
{
    toplevel,
    fullscreen,
    maximized,
    transient
};

//user data of wl_shell_surface resources is the a surfaceRes, which role is a shellSurface
class shellSurfaceRes : public resource
{
protected:
    friend surfaceRes;

    shellSurfaceRes(surfaceRes* surf, wl_client* client, unsigned int id);

    std::string class_;
    std::string title_;

    shellSurfaceState state_ = shellSurfaceState::toplevel;

    vec2ui position_;
public:
    std::string getClass() const { return class_; };
    std::string getTitle() const { return title_; };

    vec2ui getPosition() const { return position_; }

    shellSurfaceState getState() const { return state_; }

    resourceType getType() const { return resourceType::shellSurface; }
};
