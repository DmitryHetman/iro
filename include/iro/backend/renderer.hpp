#pragma once

#include <iro/include.hpp>
#include <nytl/nonCopyable.hpp>

namespace iro
{

//renderer
class renderer : public nonCopyable
{
public:
    virtual ~renderer() = default;

    virtual std::unique_ptr<outputRendererData> initOutput(output& outp) = 0;
    virtual std::unique_ptr<surfaceRendererData> initBuffer(buffer& buf) = 0;
};


}
