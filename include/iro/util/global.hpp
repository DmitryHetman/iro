#pragma once

#include <iro/include.hpp>
#include <nytl/nonCopyable.hpp>

namespace iro
{

///The Global class represents a wl_global. 
class Global : public nytl::NonCopyable
{
protected:
    wl_global* wlGlobal_;

public:
    Global(wl_global* gl = nullptr) : wlGlobal_(gl) {}
    ~Global();

    wl_global* wlGlobal() const { return wlGlobal_; }
};

}
