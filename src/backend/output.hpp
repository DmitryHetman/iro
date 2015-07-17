#pragma once

#include <iro.hpp>
#include <resources/resource.hpp>

#include <util/nonCopyable.hpp>
#include <util/vec.hpp>

#include <vector>

output* outputAt(int x, int y);
output* outputAt(vec2i pos);

class output : public nonCopyable
{
protected:
    unsigned int id_;

    std::vector<surfaceRes*> mappedSurfaces_;

    wl_event_source* drawEventSource_;
    wl_global* global_;

    friend int outputRedraw(void* data);
    virtual void render();

public:
    output(unsigned int id);
    virtual ~output();

    const std::vector<surfaceRes*> getSurfaces() const { return mappedSurfaces_; }

    void mapSurface(surfaceRes* surf);
    void unmapSurface(surfaceRes* surf);

    virtual void refresh();

    surfaceRes* getSurfaceAt(vec2i pos);

    virtual void swapBuffers() = 0;
    virtual void makeEglCurrent() = 0;

    virtual vec2ui getSize() const = 0;
    virtual vec2i getPosition() const { return vec2i(); }
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
