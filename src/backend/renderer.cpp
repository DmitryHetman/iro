#include <backend/renderer.hpp>
#include <resources/surface.hpp>
#include <resources/buffer.hpp>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdexcept>

/////////////////////////////////////////
surfaceProgram::surfaceProgram()
{
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
}

surfaceProgram::~surfaceProgram()
{
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
}

void surfaceProgram::use(rect2f geometry, unsigned int texture, bufferFormat format)
{

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
}

renderer::~renderer()
{

}

bool renderer::render(surfaceRes* surface)
{
    if(!surface || surface->getRole() == surfaceRole::none || !surface->getPending().attached)
    {
        return 0;
    }

    bufferRes* buff = surface->getPending().attached;
    if(!buff->initialized())
    {
        if(!buff->init())
            return 0;
    }

    shader* prog;

    if(buff->getFormat() == bufferFormat::argb32)
        prog = &shader::argb;
    else if(buff->getFormat() == bufferFormat::rgb32)
        prog = &shader::rgb;

    prog->use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, buff->getTexture());

    glUniform1i(glGetUniformLocation(prog->getProgram(), "texture0"), 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    const float coords[] = {
        0.5f,  -0.5f,
        -0.5f,  -0.5f,
        0.5f, 0.5f,
        -0.5f, 0.5f,
    };

    const float uv[] = {
        1, 0,
        0, 0,
        1, 1,
        0, 1
    };

    GLint ppos = glGetAttribLocation(prog->getProgram(), "pos");
    GLint puv = glGetAttribLocation(prog->getProgram(), "uv");

    glEnableVertexAttribArray(ppos);
    glEnableVertexAttribArray(puv);

    glVertexAttribPointer(ppos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*) coords);
    glVertexAttribPointer(puv, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*) uv);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    return 1;
}

bool renderer::drawCursor()
{
    if(!cursor_)
        return 0;

    shader* prog = &shader::rgb;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cursor_->getTexture());


    const float coords[] = {
        0.5f,  -0.5f,
        -0.5f,  -0.5f,
        0.5f, 0.5f,
        -0.5f, 0.5f,
    };

    const float uv[] = {
        1, 0,
        0, 0,
        1, 1,
        0, 1
    };

    GLint ppos = glGetAttribLocation(prog->getProgram(), "pos");
    GLint puv = glGetAttribLocation(prog->getProgram(), "uv");

    glEnableVertexAttribArray(ppos);
    glEnableVertexAttribArray(puv);

    glVertexAttribPointer(ppos, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*) coords);
    glVertexAttribPointer(puv, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*) uv);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    return 1;
}

