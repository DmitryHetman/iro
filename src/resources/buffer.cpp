#include <resources/buffer.hpp>

#include <backend/egl.hpp>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <EGL/egl.h>

#include <wayland-server-protocol.h>

#include <iostream>

buffer::buffer(wl_resource* resource) : buffer_(resource), type_(bufferType::unknown)
{

}

buffer::~buffer()
{

}

bool buffer::initialize()
{
    wl_shm_buffer* shmBuffer = nullptr;
    shmBuffer = wl_shm_buffer_get(buffer_);

    if(shmBuffer) //buffer is shm
    {
        std::cout << "attaching shm" << std::endl;
        unsigned int format = wl_shm_buffer_get_format(shmBuffer);

        if(format == WL_SHM_FORMAT_ARGB8888)
        {
            format_ = bufferFormat::argb32;
        }
        else if(format == WL_SHM_FORMAT_XRGB8888)
        {
            format_ = bufferFormat::rgb32;
        }
        else
        {
            format_ = bufferFormat::unknown;
            return false;
        }

        unsigned int pitch = wl_shm_buffer_get_stride(shmBuffer) / 4;
        GLint glFormat = GL_BGRA_EXT;
        GLint glPixel = GL_UNSIGNED_BYTE;

        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &textures_[0]);
        glBindTexture(GL_TEXTURE_2D, textures_[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glPixelStorei(GL_UNPACK_ROW_LENGTH_EXT, pitch);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS_EXT, 0);
        glPixelStorei(GL_UNPACK_SKIP_ROWS_EXT, 0);

        wl_shm_buffer_begin_access(shmBuffer);
        void* data = wl_shm_buffer_get_data(shmBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, glFormat, pitch, wl_shm_buffer_get_height(shmBuffer), 0, glFormat, glPixel, data);
        wl_shm_buffer_end_access(shmBuffer);

        type_ = bufferType::shm;
    }
    else //buffer is egl
    {
        std::cout << "attaching egl" << std::endl;
        EGLint format;

        eglContext* ctx = getEglContext();
        if(!eglContext::eglQueryWaylandBufferWL(ctx->getDisplay(), buffer_, EGL_TEXTURE_FORMAT, &format))
        {
            //not a egl buffer
            return false;
        }
    }

    initialized_ = 1;
    return true;
}
