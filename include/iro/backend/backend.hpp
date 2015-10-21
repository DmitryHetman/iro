#pragma once

#include <iro/include.hpp>
#include <nyutil/nonCopyable.hpp>
#include <nyutil/callback.hpp>

#include <vector>

namespace backendType //not enum, can be extended
{
    const unsigned char none = 0;
    const unsigned char x11 = 1;
    const unsigned char kms = 2;
}

class backend : public nonCopyable
{
protected:
    std::vector<output*> outputs_;

    callback<void(output&)> outputCreatedCallback_;
    callback<void(output&)> outputDestroyedCallback_;

public:
    virtual ~backend();

    std::vector<output*> getOutputs() const { return outputs_; }

    std::unique_ptr<connection> onOutputCreated(std::function<void(output&)> func){ return outputCreatedCallback_.add(func); }
    std::unique_ptr<connection> onOutputDestroyed(std::function<void(output&)> func){ return outputDestroyedCallback_.add(func); }

    virtual unsigned int getType() const = 0;
    virtual void* getNativeDisplay() const = 0;
};
