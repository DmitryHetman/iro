#include <iro/compositor/buffer.hpp>

#include <iro/backend/egl.hpp>
#include <iro/util/log.hpp>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <EGL/egl.h>

#include <wayland-server-protocol.h>

#include <iostream>

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
{
    unsigned int format = wl_shm_buffer_get_format(shmBuffer);

    switch(format)
    {
        case WL_SHM_FORMAT_ARGB8888: format_ = bufferFormat::argb32; break;
        case WL_SHM_FORMAT_XRGB8888: format_ = bufferFormat::rgb32; break;
        default: return false;
    }

    unsigned int pitch = wl_shm_buffer_get_stride(shmBuffer) / 4;
    GLint glFormat = GL_BGRA_EXT;
    GLint glPixel = GL_UNSIGNED_BYTE;

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ROW_LENGTH_EXT, pitch);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS_EXT, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS_EXT, 0);

    wl_shm_buffer_begin_access(shmBuffer);
    void* data = wl_shm_buffer_get_data(shmBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, glFormat, pitch, wl_shm_buffer_get_height(shmBuffer), 0, glFormat, glPixel, data);
    wl_shm_buffer_end_access(shmBuffer);

    return true;
}

bool bufferRes::fromEglBuffer(wl_resource* buffer)
{
    std::cout << "from egl" << std::endl;

    EGLint format;

    eglContext* ctx = iroEglContext();
    if(!eglContext::eglQueryWaylandBufferWL(ctx->getDisplay(), buffer, EGL_TEXTURE_FORMAT, &format))
    {
        //not a egl buffer
        return false;
    }

    switch(format)
    {
        default:format_ = bufferFormat::argb32; break;
    }

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    EGLint attribs[] =
    {
        EGL_WAYLAND_PLANE_WL, 0,
        EGL_NONE
    };

    image_ = iroEglContext()->eglCreateImageKHR(iroEglContext()->getDisplay(), iroEglContext()->getContext(), EGL_WAYLAND_BUFFER_WL, (EGLClientBuffer) wlResource_, attribs);

    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC func = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) eglGetProcAddress("glEGLImageTargetTexture2DOES");
    func(GL_TEXTURE_2D, image_);

    return true;
}
*/
