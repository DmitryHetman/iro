#pragma once

#include <iro.hpp>
#include <util/nonCopyable.hpp>

#include <resources/resource.hpp>

class subcompositor : public nonCopyable
{
public:
    subcompositor();
};

//////////////////////
class subcompositorRes : public resource
{
public:
    subcompositorRes(wl_client* client, unsigned int id, unsigned int version);
};
