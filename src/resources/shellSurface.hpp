#pragma once

#include <iro.hpp>
#include <resources/resource.hpp>
#include <util/vec.hpp>

#include <string>

//todo: state
enum class shellSurfaceState
{
    toplevel,
    fullscreen,
    maximized,
    transient
};

class shellSurfaceRes : public resource
{
protected:
    friend surfaceRes;
    shellSurfaceRes(surfaceRes& surf, wl_client& client, unsigned int id);

    std::string className_;
    std::string title_;

    bool ping_ = 0;

    surfaceRes& surface_;

    shellSurfaceState state_ = shellSurfaceState::toplevel;

    union //different states
    {
        vec2i toplevelPosition_;
        output* fullscreenOutput_;
    };

public:
    surfaceRes& getSurface() const { return surface_; }

    std::string getClassName() const { return className_; };
    std::string getTitle() const { return title_; };

    void setClassName(const std::string& name);
    void setTitle(const std::string& name);

    void ping();
    void pong() { ping_ = 0; }

    bool hasPing() const { return ping_; }

    void move(vec2i delta){ toplevelPosition_ += delta; }

    shellSurfaceState getState() const { return state_; }

    vec2i getToplevelPosition() const { return state_ == shellSurfaceState::toplevel ? toplevelPosition_ : vec2i(); }
    output* getFullscreenOutput() const { return state_ == shellSurfaceState::fullscreen ? fullscreenOutput_ : nullptr; }

    //resource
    resourceType getType() const { return resourceType::shellSurface; }
};
