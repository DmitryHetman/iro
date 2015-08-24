#pragma once

#include <iro/include.hpp>
#include <iro/util/iroDrawContext.hpp>
#include <nyutil/nonCopyable.hpp>

class renderData
{
public:
    virtual ~renderData(){}
};

class renderer : public nonCopyable
{
protected:
<<<<<<< HEAD
    void setBufferSize(bufferRes& buff, vec2ui size) const;
    void setBufferData(bufferRes& buff, bufferData* data) const;
    void setBufferFormat(bufferRes& buff, bufferFormat format) const;
    bufferData* getBufferData(bufferRes& buff) const;
=======
    void setOutputData(output& o, renderData* data) const;
    void setSurfaceData(surfaceRes& surf, renderData* data) const;

    renderData* getOutputData(output& o) const;
    renderData* getSurfaceData(surfaceRes& surf) const;
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527

public:
    renderer();
    virtual ~renderer();

    virtual iroDrawContext& getDrawContext(output& o) = 0; //for direct drawing on outputs
    virtual void attachSurface(surfaceRes& surf, bufferRes& buf) = 0;

    virtual void initOutput(output& o) = 0;
    virtual void uninitOutput(output& o) = 0;

    virtual void applyOutput(output& o) = 0;
};
