#include <iro/backend/glRenderer.hpp>
#include <iro/backend/egl.hpp>
#include <iro/backend/output.hpp>
#include <iro/compositor/surface.hpp>
#include <iro/compositor/buffer.hpp>
#include <iro/seat/pointer.hpp>

#include <iro/util/shaderSources.hpp>
#include <iro/util/log.hpp>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <cassert>

////////////////////////////////
rect2f rectToGL(const rect2f& src, const vec2ui& size)
{
    rect2f ret;

    ret.position = (vec2f(src.position.x, size.y - src.position.y - src.size.y) / size) * 2 - 1;
    ret.size = (src.size / size) * 2;

    return ret;
}


/////////////////////////////////////////////////////7
glRenderer::glRenderer(eglContext& ctx)
{
    if(!ctx.isCurrent())
    {
        throw std::runtime_error("glRenderer::glRenderer: invalid eglContext");
        return;
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
    //init cursor texture
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &defaultCursorTex_);
    glBindTexture(GL_TEXTURE_2D, defaultCursorTex_);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ROW_LENGTH_EXT, 1);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS_EXT, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS_EXT, 0);

    GLfloat pixels[] = {
        1.0f, 1.0f, 1.0f
    };

    glTexImage2D(GL_TEXTURE_2D, 0,  GL_RGB, 1, 1, 0, GL_RGB, GL_FLOAT, pixels);
}

glRenderer::~glRenderer()
{
    glDeleteTextures(1, &defaultCursorTex_);
}

void glRenderer::drawTex(rect2f geometry, unsigned int texture, bufferFormat format)
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

    shader* program = nullptr;
    switch(format)
    {
        case bufferFormat::argb32: program = &argbShader_; break;
        case bufferFormat::rgb32: program = &rgbShader_; break;
        default: return;
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
}

bool glRenderer::render(surfaceRes& surface, vec2i pos)
{
    //check surface
    if(surface.getRole() == surfaceRole::none || !surface.getCommited().attached)
    {
        iroWarning("renderer::render: ", "invalid surface state");
        return 0;
    }

    //init buffer
    bufferRes* buff = surface.getCommited().attached;
    if(!buff->initialized())
    {
        if(!buff->init())
        {
            iroWarning("renderer::render: ", "could not init bufferRes");
            return 0;
        }
    }

    //render
    rect2f geometry = surface.getExtents();
    geometry.position += pos;

    drawTex(geometry, buff->getTexture(), buff->getFormat());

    return 1;
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
        drawTex(geometry, defaultCursorTex_, bufferFormat::rgb32);
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
