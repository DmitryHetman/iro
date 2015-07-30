#include <iro/compositor/buffer.hpp>

#include <iro/backend/egl.hpp>
#include <iro/util/log.hpp>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <EGL/egl.h>

#include <wayland-server-protocol.h>

#include <iostream>

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
