#pragma once

#include <iro/include.hpp>
#include <iro/backend/renderer.hpp>
#include <ny/gl/shader.hpp>
#include <nyutil/rect.hpp>

class surfaceDataGL;
class outputDataGL;

class bufferDataGL;

class glRenderer : public renderer
{
protected:
    ny::shader argbShader_;
    ny::shader rgbShader_;

<<<<<<< HEAD
    unsigned int defaultCursorTex_ = 0;

    bool drawTex(rect2f geometry, unsigned int texture, bufferFormat format);
    bufferDataGL* initBuffer(bufferRes& buff);
=======
    outputDataGL* getOutputData(output& o) const;
    surfaceDataGL* getSurfaceData(surfaceRes& surf) const;
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527

public:
    glRenderer();
    virtual ~glRenderer();

    virtual iroDrawContext& getDrawContext(output& o) override;
    virtual void attachSurface(surfaceRes& surf, bufferRes& buf) override;

    virtual void initOutput(output& o) override;
    virtual void uninitOutput(output& o) override;

    virtual void applyOutput(output& o) override;
};
