#pragma once

#include <iro/include.hpp>
#include <iro/backend/renderer.hpp>
#include <ny/gl/shader.hpp>
#include <nyutil/rect.hpp>

class surfaceDataGL;
class outputDataGL;

class glRenderer : public renderer
{
protected:
    ny::shader argbShader_;
    ny::shader rgbShader_;

    outputDataGL* getOutputData(output& o) const;
    surfaceDataGL* getSurfaceData(surfaceRes& surf) const;

public:
    glRenderer();
    virtual ~glRenderer();

    virtual iroDrawContext& getDrawContext(output& o) override;
    virtual void attachSurface(surfaceRes& surf, bufferRes& buf) override;

    virtual void initOutput(output& o) override;
    virtual void uninitOutput(output& o) override;

    virtual void applyOutput(output& o) override;
};
