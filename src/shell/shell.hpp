#pragma once

#include <iro.hpp>
#include <util/nonCopyable.hpp>

#include <resources/resource.hpp>

class shell : public nonCopyable
{
protected:
    wl_global* global_;

public:
    shell();
    ~shell();
};

//////////////////////////////7
class shellRes : public resource
{
public:
    shellRes(wl_client* client, unsigned int id, unsigned int version);

    resourceType getType() const { return resourceType::shell; }
};
