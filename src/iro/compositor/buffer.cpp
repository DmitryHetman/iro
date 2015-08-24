#include <iro/compositor/buffer.hpp>

#include <iro/backend/egl.hpp>
#include <iro/util/log.hpp>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <EGL/egl.h>

#include <wayland-server-protocol.h>

#include <iostream>

<<<<<<< HEAD
unsigned int getBufferFormatSize(bufferFormat format)
{
    switch(format)
    {
        case bufferFormat::xrgb32: case bufferFormat::argb32: return 4;
        case bufferFormat::rgb24: return 3;
        default: return 0;
    }
}

bufferData::~bufferData()
{
}

////////////////////////////////////////////////////////////////////
//listener callback function, when a buffer resource was destroyed
void destroyBuffer(wl_listener* listener, void* data)
{
    bufferResPOD* buffer;
    buffer = wl_container_of(listener, buffer, destroyListener);

    delete buffer->buffer;
}

////////////////////////////////////////////////////////////////////////////////////////
bufferRes* bufferForResource(wl_resource& res)
{
    wl_listener* listener = wl_resource_get_destroy_listener(&res, &destroyBuffer);
    if(listener)
    {
        bufferResPOD* buffer;
        buffer = wl_container_of(listener, buffer, destroyListener);
        return buffer->buffer;
    }

    return new bufferRes(res);
}

////////////////////
bufferRes::bufferRes(wl_resource& res) : resource(res)
{
    wl_resource_add_destroy_listener(&res, &pod_.destroyListener);
    pod_.destroyListener.notify = &destroyBuffer;

    pod_.buffer = this;
}

bufferRes::~bufferRes()
{
    std::cout << "deleted buffer " << this << std::endl;
}

/*
bool bufferRes::fromShmBuffer(wl_shm_buffer* shmBuffer)
=======
////////////////////////////////////////////////////////////////////
//listener callback function, when a buffer resource was destroyed
void destroyBuffer(wl_listener* listener, void* data)
{
    bufferResPOD* buffer;
    buffer = wl_container_of(listener, buffer, destroyListener);

    delete buffer->buffer;
}

////////////////////////////////////////////////////////////////////////////////////////
bufferRes& bufferForResource(wl_resource& res)
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527
{
    wl_listener* listener = wl_resource_get_destroy_listener(&res, &destroyBuffer);
    if(listener)
    {
        bufferResPOD* buffer;
        buffer = wl_container_of(listener, buffer, destroyListener);
        return *buffer->buffer;
    }

    bufferRes* ret = new bufferRes(res);
    return *ret;
}
<<<<<<< HEAD
*/
=======

////////////////////
bufferRes::bufferRes(wl_resource& res) : resource(res)
{
    wl_resource_add_destroy_listener(&res, &pod_.destroyListener);
    pod_.destroyListener.notify = &destroyBuffer;

    pod_.buffer = this;
}

bufferRes::~bufferRes()
{
}
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527
