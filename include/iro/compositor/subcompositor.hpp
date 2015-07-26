#pragma once

#include <iro/include.hpp>
#include <nyutil/nonCopyable.hpp>

#include <iro/compositor/resource.hpp>

class subcompositor : public nonCopyable
{
public:
    subcompositor();
};

//////////////////////
class subcompositorRes : public resource
{
public:
    subcompositorRes(wl_client& client, unsigned int id, unsigned int version);
};
