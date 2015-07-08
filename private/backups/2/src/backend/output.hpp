#pragma once

#include <iro.hpp>
#include <resources/resource.hpp>
#include <util/nonCopyable.hpp>

#include <vector>

class output : public nonCopyable
{
protected:
    std::vector<surfaceRes*> surfaces_;
    renderer* renderer_ = nullptr;

public:
    output();
    virtual ~output();

    const std::vector<surfaceRes*> getSurfaces() const { return surfaces_; }

    void mapSurface(surfaceRes* surf);
    void unmapSurface(surfaceRes* surf);

    virtual void render();

    virtual void swapBuffers() = 0;
    virtual void makeEglCurrent() = 0;
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
