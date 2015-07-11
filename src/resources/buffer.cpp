#include <resources/buffer.hpp>

#include <backend/egl.hpp>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <EGL/egl.h>

#include <wayland-server-protocol.h>

#include <iostream>

bufferRes::bufferRes(wl_resource* res) : resource(res)
{
}

bufferRes::~bufferRes()
{
    if(texture_) glDeleteTextures(1, &texture_);
}

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
    std::cout << "attaching egl" << std::endl;
    EGLint format;

    eglContext* ctx = getEglContext();
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

    image_ = getEglContext()->eglCreateImageKHR(getEglContext()->getDisplay(), getEglContext()->getContext(), EGL_WAYLAND_BUFFER_WL, (EGLClientBuffer) wlResource_, attribs);

    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC func = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) eglGetProcAddress("glEGLImageTargetTexture2DOES");
    func(GL_TEXTURE_2D, image_);

    return true;
}

bool bufferRes::init()
{
    if(!wlResource_)
        return 0;

    if(initialized())
        return 1;

    std::cout << wlResource_ << std::endl;

    wl_shm_buffer* shmBuffer = wl_shm_buffer_get(wlResource_);
    if(shmBuffer)
    {
        type_ = bufferType::shm;
        return fromShmBuffer(shmBuffer);
    }
    else
    {
        type_ = bufferType::egl;
        return fromEglBuffer(wlResource_);
    }
}

vec2ui bufferRes::getSize() const
{
    vec2ui ret;
    if(type_ == bufferType::shm)
    {
        wl_shm_buffer* shmBuffer = wl_shm_buffer_get(wlResource_);
        ret.x = wl_shm_buffer_get_width(shmBuffer);
        ret.y = wl_shm_buffer_get_height(shmBuffer);
    }
    else if(type_ == bufferType::egl)
    {
        int w, h;
        getEglContext()->eglQueryWaylandBufferWL(getEglContext()->getDisplay(), wlResource_, EGL_WIDTH, &w);
        getEglContext()->eglQueryWaylandBufferWL(getEglContext()->getDisplay(), wlResource_, EGL_HEIGHT, &h);
        ret.x = w;
        ret.y = h;
    }

    return ret;
}

