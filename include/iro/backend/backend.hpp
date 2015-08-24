#pragma once

#include <iro/include.hpp>
#include <nyutil/nonCopyable.hpp>

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

public:
    backend();
    virtual ~backend();

    std::vector<output*> getOutputs() const { return outputs_; }
    wl_event_source* getWlEventSource() const { return wlEventSource_; }

    virtual backendType getType() const = 0;
    virtual void* getEGLDisplay() const = 0;
};
