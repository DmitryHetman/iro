#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>

#include <nyutil/nonCopyable.hpp>
#include <nyutil/vec.hpp>
<<<<<<< HEAD

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
=======
#include <ny/surface.hpp>

#include <wayland-server-core.h>

bufferRes& bufferForResource(wl_resource& res);

//for wl_container_of
class bufferResPOD
{
public:
    wl_listener destroyListener;
    bufferRes* buffer;
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527
};

///////////////////////////////////////////////
class bufferRes : public resource
{
<<<<<<< HEAD

friend class renderer;

protected:
    bufferResPOD pod_;

    bufferFormat format_ = bufferFormat::unknown;
    vec2ui size_;

    bufferData* data_ = nullptr;
=======

friend class renderer;

protected:
    bufferResPOD pod_;
    ny::bufferFormat format_ = ny::bufferFormat::unknown;
    vec2ui size_;
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527

public:
    bufferRes(wl_resource& res);
    virtual ~bufferRes();

<<<<<<< HEAD
    bufferFormat getFormat() const { return format_; }
    vec2ui getSize() const { return size_; }

    void setReinit()
    {
        if(data_) delete data_;
        data_ = nullptr;
    }
=======
    ny::bufferFormat getFormat() const { return format_; }
    vec2ui getSize() const { return size_; }
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527

    //res
    virtual resourceType getType() const override { return resourceType::buffer; }
};

<<<<<<< HEAD
bufferRes* bufferForResource(wl_resource& res); //gets the bufferRes for the given resource; if non-existent creates it
=======

>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527

