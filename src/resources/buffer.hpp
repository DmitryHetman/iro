#pragma once

#include <iro.hpp>
#include <util/nonCopyable.hpp>

#include <wayland-server-core.h>

enum class bufferType
{
    unknown = 0,

    shm,
    egl
};

enum class bufferFormat
{
    unknown = 0,

    rgb32,
    argb32
};

class buffer : public nonCopyable
{
protected:
    wl_resource* buffer_;

    bufferFormat format_;
    bool initialized_;

    bufferType type_;
    unsigned int textures_[3] = {0, 0, 0};
    void* images_[3] = {nullptr, nullptr, nullptr};

public:
    buffer(wl_resource* resource);
    ~buffer();

    bool initialized() const { return initialized_; }
    bool initialize();

    bufferType getType() const { return type_; }
    bufferFormat getFormat() const { return format_; }

    void* getImage(unsigned int i = 0) const { return images_[i]; }
    unsigned int getTexture(unsigned int i = 0) const { return textures_[i]; }

    wl_resource* getWlResource() const { return buffer_; }
};
