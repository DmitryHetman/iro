#pragma once

#include <iro.hpp>
#include <resources/resource.hpp>

#include <util/nonCopyable.hpp>
#include <util/vec.hpp>

#include <vector>

class output : public nonCopyable
{
protected:
    unsigned int id_;

    std::vector<surfaceRes*> mappedSurfaces_;
    renderer* renderer_ = nullptr;

    wl_event_source* drawEventSource_;
    wl_global* global_;

public:
    output(unsigned int id);
    virtual ~output();

    const std::vector<surfaceRes*> getSurfaces() const { return mappedSurfaces_; }

    void mapSurface(surfaceRes* surf);
    void unmapSurface(surfaceRes* surf);

    virtual void render();
    virtual void refresh();

    surfaceRes* getSurfaceAt(int x, int y) const { return nullptr; }

    virtual void swapBuffers() = 0;
    virtual void makeEglCurrent() = 0;

    virtual vec2ui getSize() const = 0;
};

//////////////////
class outputRes : public resource
{
protected:
    output* output_;

public:
    outputRes(output* out, wl_client* client, unsigned int id, unsigned int version);

    output* getOutput() const { return output_; }
};
