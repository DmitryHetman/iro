#pragma once

#include <wayland-server-core.h>
#include <utils/rect.hpp>
#include <utils/vec.hpp>
using namespace utils;

struct surfaceState
{
    rect2i opaque = vec2i(-1,-1);
    rect2i input = vec2i(-1,-1);
    rect2i damage = vec2i(-1,-1);
    vec2i offset = vec2i(0, 0);
    wl_resource* buffer = 0;
    int scale = 1;
    bool attached = 0;
};

class surface
{
protected:
    surfaceState current_;
    surfaceState pending_;
    wl_resource* resource_;
public:
    surface(wl_resource* res);

    surfaceState& getPending(){ return pending_; }
    const surfaceState& getCurrent() const { return current_; }
    void commit();
};

class shellSurface
{

};

class client
{

};
