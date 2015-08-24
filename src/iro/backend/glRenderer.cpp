#include <iro/backend/glRenderer.hpp>
#include <iro/backend/egl.hpp>
#include <iro/backend/output.hpp>
#include <iro/compositor/surface.hpp>
#include <iro/compositor/buffer.hpp>
#include <iro/backend/backend.hpp>
#include <iro/seat/pointer.hpp>

#include <iro/util/shaderSources.hpp>
#include <iro/util/log.hpp>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <wayland-server-protocol.h>
#include <wayland-server-core.h>
#include <cassert>

<<<<<<< HEAD
class bufferDataGL : public bufferData
{
public:
    virtual ~bufferDataGL()
    {
        //if(texture_) glDeleteTextures(1, &texture_);
        //if(eglImage_) iroEglContext()->eglDestroyImageKHR(iroEglContext()->getDisplay(), eglImage_);
=======
class surfaceDataGL : public renderData
{
public:
    virtual ~surfaceDataGL()
    {
        if(texture_) glDeleteTextures(1, &texture_);
        if(eglImage_) iroEglContext()->eglDestroyImageKHR(iroEglContext()->getDisplay(), eglImage_);
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527
    }

    unsigned int texture_;
    void* eglImage_;
};
<<<<<<< HEAD
=======

class outputDataGL : public renderData
{
public:
    outputDataGL(output& o)
    {
        eglSurface_ = eglCreateWindowSurface(iroEglContext()->getDisplay(), iroEglContext()->getConfig(), (EGLNativeWindowType) o.getNativeSurface(), nullptr);
    }

    virtual ~outputDataGL()
    {
        if(eglSurface_) eglDestroySurface(iroEglContext()->getDisplay(), eglSurface_);
    }

    EGLSurface eglSurface_ = nullptr;
};
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527
////////////////////////////////
rect2f rectToGL(const rect2f& src, const vec2ui& size)
{
    rect2f ret;

    ret.position = (vec2f(src.position.x, size.y - src.position.y - src.size.y) / size) * 2 - 1;
    ret.size = (src.size / size) * 2;

    return ret;
}


/////////////////////////////////////////////////////7
glRenderer::glRenderer()
{
    if(!iroEglContext())
    {
        throw std::runtime_error("glRenderer::glRenderer: invalid eglContext");
        return;
    }

    outputDataGL* data = nullptr;
    if(!iroEglContext()->isCurrent())
    {
        if(!iroBackend()->getOutputs()[0] || (!data = getOutputData(*iroBackend()->getOutputs()[0])))
        {
            throw std::runtime_error("glRenderer::glRenderer: no valid output to make eglContext current");
            return;
        }

        if(!iroEglContext()->makeCurrent(data->eglSurface_))
        {
            throw std::runtime_error("glRenderer::glRenderer: failed to make egl context current");
            return;
        }
    }

    std::string version;
    version.append((const char*) glGetString(GL_VERSION));
    iroLog("glVersion: ", version);

    //init shader
    if(!argbShader_.loadFromString(sourceVS, argbFS) ||
       !rgbShader_.loadFromString(sourceVS, rgbFS))
    {
        throw std::runtime_error("could not initialize shaders");
        return;
    }
}

glRenderer::~glRenderer()
{
}

iroDrawContext& glRenderer::getDrawContext(output& o)
{

}
void glRenderer::attachSurface(surfaceRes& surf, bufferRes& buf)
{

}

void glRenderer::initOutput(output& o)
{

}
void glRenderer::uninitOutput(output& o)
{

<<<<<<< HEAD
bool glRenderer::drawTex(rect2f geometry, unsigned int texture, bufferFormat format)
=======
}

void glRenderer::applyOutput(output& o)
{

}
/*
bool glRenderer::drawTex(rect2f geometry, unsigned int texture, ny::bufferFormat format)
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527
{
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    rect2f glGeometry = rectToGL(geometry, vec2ui(viewport[2], viewport[3]));

    const GLfloat vertices[] =
    {
        glGeometry.topLeft().x, glGeometry.topLeft().y,
        glGeometry.topRight().x, glGeometry.topRight().y,
        glGeometry.bottomRight().x, glGeometry.bottomRight().y,
        glGeometry.bottomLeft().x, glGeometry.bottomLeft().y,
    };

    const GLfloat uv[] =
    {
        0, 0,
        1, 0,
        1, 1,
        0, 1
    };

    ny::shader* program = nullptr;
    switch(format)
    {
        case bufferFormat::argb32: program = &argbShader_; break;
        case bufferFormat::xrgb32: program = &rgbShader_; break;
        case bufferFormat::rgb24: program = &rgbShader_; break;
        default: return 0;
    }

    program->use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*) vertices);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*) uv);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    return 1;
}

bufferDataGL* glRenderer::initBuffer(bufferRes& buff)
{
    assert(iroEglContext());

    wl_resource& res = buff.getWlResource();
    eglContext* ctx = iroEglContext();

    unsigned int texture = 0;
    void* image = nullptr;

    wl_shm_buffer* shm;
    int eglFormat;

    shm = wl_shm_buffer_get(&res);
    if(shm) //is shmBuffer
    {
        //format
        unsigned int format = wl_shm_buffer_get_format(shm);
<<<<<<< HEAD
        bufferFormat buffFormat;

        switch(format)
        {
            case WL_SHM_FORMAT_ARGB8888: buffFormat = bufferFormat::argb32; break;
            case WL_SHM_FORMAT_XRGB8888: buffFormat = bufferFormat::xrgb32; break;
            case WL_SHM_FORMAT_RGB888: buffFormat = bufferFormat::rgb24; break;
=======
        ny::bufferFormat buffFormat;

        switch(format)
        {
            case WL_SHM_FORMAT_ARGB8888: buffFormat = ny::bufferFormat::argb32; break;
            case WL_SHM_FORMAT_XRGB8888: buffFormat = ny::bufferFormat::xrgb32; break;
            case WL_SHM_FORMAT_RGB888: buffFormat = ny::bufferFormat::rgb24; break;
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527
            default: return nullptr;
        }

        setBufferFormat(buff, buffFormat);

        //size
        vec2ui size;
        size.x = wl_shm_buffer_get_width(shm);
        size.y = wl_shm_buffer_get_height(shm);
        setBufferSize(buff, size);

        //texture
        unsigned int pitch = wl_shm_buffer_get_stride(shm) / getBufferFormatSize(buffFormat);
        GLint glFormat = GL_BGRA_EXT;
        GLint glPixel = GL_UNSIGNED_BYTE;

        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glPixelStorei(GL_UNPACK_ROW_LENGTH_EXT, pitch);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS_EXT, 0);
        glPixelStorei(GL_UNPACK_SKIP_ROWS_EXT, 0);

        //data
        wl_shm_buffer_begin_access(shm);
        glTexImage2D(GL_TEXTURE_2D, 0, glFormat, pitch, size.y, 0, glFormat, glPixel, wl_shm_buffer_get_data(shm));
        wl_shm_buffer_end_access(shm);
    }
    else if(iroEglContext()->eglQueryWaylandBufferWL(ctx->getDisplay(), &res, EGL_TEXTURE_FORMAT, &eglFormat)) //is egl buffer
    {
        //format
        bufferFormat buffFormat;

        switch(eglFormat)
        {
<<<<<<< HEAD
            case EGL_TEXTURE_RGBA: buffFormat = bufferFormat::argb32; break;
            case EGL_TEXTURE_RGB: buffFormat = bufferFormat::rgb24; break;
=======
            case EGL_TEXTURE_RGBA: ny::buffFormat = bufferFormat::argb32; break;
            case EGL_TEXTURE_RGB: ny::buffFormat = bufferFormat::rgb24; break;
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527
            default: return nullptr;
        }

        setBufferFormat(buff, buffFormat);

        //size
        vec2i size;
        iroEglContext()->eglQueryWaylandBufferWL(iroEglContext()->getDisplay(), &res, EGL_WIDTH, &size.x);
        iroEglContext()->eglQueryWaylandBufferWL(iroEglContext()->getDisplay(), &res, EGL_HEIGHT, &size.y);
        setBufferSize(buff, size);

        //texture
        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        //data
        EGLint attribs[] =
        {
            EGL_WAYLAND_PLANE_WL, 0,
            EGL_NONE
        };

        image = ctx->eglCreateImageKHR(iroEglContext()->getDisplay(), iroEglContext()->getContext(), EGL_WAYLAND_BUFFER_WL, (EGLClientBuffer) &res, attribs);
        PFNGLEGLIMAGETARGETTEXTURE2DOESPROC func = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) eglGetProcAddress("glEGLImageTargetTexture2DOES");

        if(!func)
        {
            //clean up
            return nullptr;
        }

        func(GL_TEXTURE_2D, image);

        //destroy image?
    }
    else //unknown buffer
    {
        iroWarning("glRenderer::initBuffer: invalid buffer type");
        return nullptr;
    }

    //store/ return
    bufferDataGL* data = new bufferDataGL();
    data->eglImage_ = image;
    data->texture_ = texture;

    setBufferData(buff, data);

    return data;
}

bool glRenderer::render(surfaceRes& surface, vec2i pos)
{
    //check surface
    if(surface.getRole() == surfaceRole::none || !surface.getCommited().attached.get())
    {
        iroWarning("glRenderer::render: ", "invalid surface state");
        return 0;
    }

    //init buffer
    bufferRes* buff = surface.getCommited().attached.get();
    if(!buff)
    {
        return 0;
    }

    std::cout << "buffer: " << buff << " " << buff->getID() << " " << buff->getSize() << std::endl;
    std::cout << "bufferData: " << getBufferData(*buff) << std::endl;

    bufferData* bufferdata = getBufferData(*buff);
    bufferDataGL* data = nullptr;

    if(bufferdata) data = dynamic_cast<bufferDataGL*>(bufferdata);

    if(!bufferdata || !data)
    {
        data = initBuffer(*buff);
        if(!data)
        {
            iroWarning("glRenderer::render: failed to init bufferRes ", buff);
            return 0;
        }
    }

    //render
    rect2f geometry = surface.getExtents();
    geometry.position += pos;

    return drawTex(geometry, data->texture_, buff->getFormat());
}

bool glRenderer::drawCursor(pointer& p)
{
    if(p.getCursor()) //client-set cursor
    {
        return render(*p.getCursor(), p.getPosition());
    }
    else //default cursor
    {
        rect2f geometry(p.getPosition(), vec2ui(20, 20));
        drawTex(geometry, defaultCursorTex_, bufferFormat::xrgb32);
        return 1;
    }
}

void glRenderer::beginDraw(output& out)
{
    assert(iroEglContext());

    if(!iroEglContext()->makeCurrent(out))
    {
        iroWarning("glRenderer::beginDraw: failed to make egl context current");
        return;
    }

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
}

void glRenderer::endDraw(output& out)
{
    out.swapBuffers();
    glFinish();
}
*/
