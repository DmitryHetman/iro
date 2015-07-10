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

public:
    backend() = default;
    virtual ~backend();

    std::vector<output*> getOutputs() const { return outputs_; }
    output* getOutput() const { return outputs_[0]; }
    wl_event_source* getWlEventSource() const { return wlEventSource_; }

    virtual eglContext* getEglContext() const = 0;
    virtual backendType getType() const = 0;
};
