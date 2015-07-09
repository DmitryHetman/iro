#pragma once

#include <iro.hpp>
#include <resources/resource.hpp>
#include <util/nonCopyable.hpp>

#include <wayland-server-core.h>

enum class bufferType : unsigned char
{
    unknown = 0,

    shm,
    egl
};

enum class bufferFormat : unsigned char
{
    unknown = 0,

    rgb32,
    argb32
};

class bufferRes : public resource
{
protected:
    bufferFormat format_ = bufferFormat::unknown;
    bufferType type_ = bufferType::unknown;

    unsigned int texture_ = 0; //GLTexture

    bool fromShmBuffer(wl_shm_buffer* buffer);
    bool fromEglBuffer(wl_resource* buffer);

public:
    bufferRes(wl_resource* res);
    virtual ~bufferRes();

    bool init();
    bool initialized() const { return (texture_ != 0); }

    bufferFormat getFormat() const { return format_; }
    bufferType getBufferType() const { return type_; }

    unsigned int getTexture() const { return texture_; }
};
