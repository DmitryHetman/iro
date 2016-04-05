#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>

#include <nytl/nonCopyable.hpp>
#include <nytl/vec.hpp>
#include <nytl/callback.hpp>
#include <nytl/observe.hpp>

namespace iro
{

///Represents a physical touch device.
class Touch : public nytl::NonCopyable
{
};

///Represents a clients proxy to a touch device.
class TouchRes : public Resource
{
};

}
