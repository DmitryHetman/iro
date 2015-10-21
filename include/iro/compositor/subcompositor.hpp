#pragma once

#include <iro/include.hpp>
#include <nyutil/nonCopyable.hpp>

#include <iro/compositor/resource.hpp>

class subcompositor : public nonCopyable
{
protected:
    wl_global* global_;

public:
    subcompositor();
    ~subcompositor();
};

//////////////////////
class subcompositorRes : public resource
{
public:
    subcompositorRes(wl_client& client, unsigned int id, unsigned int version);

    //res
    resourceType getType() const { return resourceType::subcompositor; }
};
