#pragma once

#include <iro/include.hpp>
#include <iro/resources/resource.hpp>

#include <nyutil/nonCopyable.hpp>
#include <nyutil/rect.hpp>
#include <nyutil/vec.hpp>

#include <vector>

output* outputAt(int x, int y);
output* outputAt(vec2i pos);

std::vector<output*> outputsAt(int x, int y, int w, int h);
std::vector<output*> outputsAt(vec2i pos, vec2i size);
std::vector<output*> outputsAt(rect2i ext);

class output : public nonCopyable
{
protected:
    unsigned int id_;

    std::vector<surfaceRes*> mappedSurfaces_;

    wl_event_source* drawEventSource_;
    wl_global* global_;

    vec2i position_;

    friend int outputRedraw(void* data);
    virtual void render();

public:
    output(unsigned int id);
    virtual ~output();

    const std::vector<surfaceRes*> getSurfaces() const { return mappedSurfaces_; }

    void mapSurface(surfaceRes* surf);
    void unmapSurface(surfaceRes* surf);

    surfaceRes* getSurfaceAt(vec2i pos);
    rect2i getExtents() const { return rect2i(getPosition(), getSize()); }

    //virtual
    virtual void refresh();

    virtual void swapBuffers();
    virtual void makeEglCurrent();
    virtual void* getEglSurface() const { return nullptr; }; //EGLSurface == void*

    virtual vec2ui getSize() const = 0;
    virtual vec2i getPosition() const;
};

//////////////////
class outputRes : public resource
{
protected:
    output& output_;

public:
    outputRes(output& out, wl_client& client, unsigned int id, unsigned int version);

    output& getOutput() const { return output_; }
};
