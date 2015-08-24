#pragma once

#include <iro/include.hpp>

#include <iro/backend/renderer.hpp>
#include <iro/util/shader.hpp>

class bufferDataGL;

class glRenderer : public renderer
{
protected:
    shader argbShader_;
    shader rgbShader_;

    unsigned int defaultCursorTex_ = 0;

    bool drawTex(rect2f geometry, unsigned int texture, bufferFormat format);
    bufferDataGL* initBuffer(bufferRes& buff);

public:
    glRenderer(eglContext& ctx);
    virtual ~glRenderer();

    virtual bool render(surfaceRes& surface, vec2i pos = vec2i(0,0)) override;
    virtual bool drawCursor(pointer& p) override;

    virtual void beginDraw(output& out) override;
    virtual void endDraw(output& out) override;
};
