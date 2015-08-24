#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>

#include <nyutil/nonCopyable.hpp>
#include <nyutil/vec.hpp>

#include <wayland-server-core.h>

//////////////////////////////////////////////
enum class bufferFormat : unsigned char
{
    unknown = 0,

    xrgb32,
    argb32,
    rgb24
};

unsigned int getBufferFormatSize(bufferFormat format);

////////////////////////////////////////////////////////////////////////
//renderer cann fill the buffer with sotred data (image, texture id),
//they inherit their data classes from this base

class bufferData
{
public:
    virtual ~bufferData();
};

//pod wrapper for bufferRes, uimportant for wl_container_of
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

    bufferFormat format_ = bufferFormat::unknown;
    vec2ui size_;

    bufferData* data_ = nullptr;

public:
    bufferRes(wl_resource& res);
    virtual ~bufferRes();

    bufferFormat getFormat() const { return format_; }
    vec2ui getSize() const { return size_; }

    void setReinit()
    {
        if(data_) delete data_;
        data_ = nullptr;
    }

    //res
    //virtual void destroy() override;
    virtual resourceType getType() const override { return resourceType::buffer; }
};

bufferRes* bufferForResource(wl_resource& res); //gets the bufferRes for the given resource; if non-existent creates it

