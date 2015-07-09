#pragma once

#include <iro.hpp>
#include <shader/shader.hpp>

#include <util/rect.hpp>
#include <util/nonCopyable.hpp>

class texProgram
{
public:
    texProgram();
    ~texProgram();

    void use(rect2f geometry, unsigned int texture, bufferFormat format);
};

class renderer : public nonCopyable
{
protected:
    surfaceRes* cursor_ = nullptr;
    texProgram texProgram_;

public:
    renderer();
    virtual ~renderer();

    bool render(surfaceRes* surface);
    bool drawCursor();

    void setCursor(surfaceRes* surf);
};
