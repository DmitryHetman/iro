#include <backend/renderer.hpp>
#include <resources/surface.hpp>
#include <resources/buffer.hpp>
#include <seat/pointer.hpp>
#include <log.hpp>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdexcept>

rect2f rectToGL(const rect2f& src, const vec2ui& size)
{
    rect2f ret;

    ret.position = (vec2f(src.position.x, size.y - src.position.y - src.size.y) / size) * 2 - 1;
    ret.size = (src.size / size) * 2;

    return ret;
}

/////////////////////////////////////////
texProgram::texProgram()
{
}

texProgram::~texProgram()
{
}

void texProgram::use(rect2f geometry, unsigned int texture, bufferFormat format)
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
        case bufferFormat::argb32: program = &shader::argb; break;
        case bufferFormat::rgb32: program = &shader::rgb; break;
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

/////////////////////////////////////////
renderer::renderer()
{
    if(!shader::initialized())
    {
        if(!shader::init())
        {
            throw std::runtime_error("could not initialize shaders");
            return;
        }
    }

    std::string version;
    version.append((const char*) glGetString(GL_VERSION));
    iroDebug("glVersion: ", version);

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

renderer::~renderer()
{
    glDeleteTextures(1, &defaultCursorTex_);
}

bool renderer::render(surfaceRes* surface, vec2ui pos)
{
    if(!surface || surface->getRole() == surfaceRole::none || !surface->getCommited().attached)
    {
        return 0;
    }

    bufferRes* buff = surface->getCommited().attached;
    if(!buff->initialized())
    {
        if(!buff->init())
            return 0;
    }

    rect2f geometry(surface->getCommited().offset, buff->getSize() * surface->getCommited().scale);
    geometry.position += pos;
    texProgram_.use(geometry, buff->getTexture(), buff->getFormat());

    return 1;
}

bool renderer::drawCursor(pointer* p)
{
    if(p->getCursor())
    {
        return render(p->getCursor(), p->getPosition());
    }
    else
    {
        rect2f geometry(p->getPosition(), vec2ui(20, 20));
        texProgram_.use(geometry, defaultCursorTex_, bufferFormat::rgb32);
        return 1;
    }
}
