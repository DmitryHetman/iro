#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>

#include <nyutil/nonCopyable.hpp>
#include <nyutil/vec.hpp>
#include <ny/surface.hpp>

#include <wayland-server-core.h>

bufferRes& bufferForResource(wl_resource& res);

//for wl_container_of
class bufferResPOD
{
public:
    wl_listener destroyListener;
    bufferRes* buffer;
};

///////////////////////////////////////////////
class bufferRes : public resource
{

friend class renderer;

protected:
    bufferResPOD pod_;
    ny::bufferFormat format_ = ny::bufferFormat::unknown;
    vec2ui size_;

public:
    bufferRes(wl_resource& res);
    virtual ~bufferRes();

    ny::bufferFormat getFormat() const { return format_; }
    vec2ui getSize() const { return size_; }

    //res
    virtual resourceType getType() const override { return resourceType::buffer; }
};



