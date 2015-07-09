#pragma once

#include <iro.hpp>
#include <util/nonCopyable.hpp>
#include <shader/shader.hpp>

class surfaceProgram
{
protected:
    unsigned int vao_;
    unsigned int vbo_;
public:
    surfaceProgram();
    ~surfaceProgram();

    void use(rect2f geometry, unsigned int texture, bufferFormat format);
};

class renderer : public nonCopyable
{
protected:
    bufferRes* cursor_ = nullptr;
    surfaceProgram program_;

public:
    renderer();
    virtual ~renderer();

    bool render(surfaceRes* surface);
    bool drawCursor();

    void setCursor(surfaceRes* surf);
};
