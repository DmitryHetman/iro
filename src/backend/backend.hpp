#pragma once

#include <iro.hpp>
#include <util/nonCopyable.hpp>

#include <vector>

enum class backendType : unsigned char
{
    none,

    x11,
    kms
};

class backend : public nonCopyable
{
protected:
    std::vector<output*> outputs_;

    wl_event_source* wlEventSource_ = nullptr;
    eglContext* eglContext_ = nullptr;

    renderer* renderer_ = nullptr;

public:
    backend();
    virtual ~backend();

    std::vector<output*> getOutputs() const { return outputs_; }
    wl_event_source* getWlEventSource() const { return wlEventSource_; }

    virtual eglContext* getEglContext() const { return eglContext_; }
    virtual renderer* getRenderer() const { return renderer_; }

    virtual backendType getType() const = 0;
};
