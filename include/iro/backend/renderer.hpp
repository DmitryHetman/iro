#pragma once

#include <iro/include.hpp>
#include <iro/util/shader.hpp>

#include <nyutil/rect.hpp>
#include <nyutil/nonCopyable.hpp>

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
    texProgram texProgram_;

    unsigned int defaultCursorTex_;

public:
    renderer();
    virtual ~renderer();

    bool render(surfaceRes& surface, vec2i pos = vec2i(0,0));
    bool drawCursor(pointer& p);
};
