#pragma once

#include <iro/include.hpp>

#include <nyutil/rect.hpp>
#include <nyutil/nonCopyable.hpp>

class renderer : public nonCopyable
{
public:
    renderer();
    virtual ~renderer();

    virtual bool render(surfaceRes& surface, vec2i pos = vec2i(0,0)) = 0;
    virtual bool drawCursor(pointer& p) = 0;

    virtual void beginDraw(output& out) = 0;
    virtual void endDraw(output& out) = 0;
};
