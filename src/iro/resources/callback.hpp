#pragma once

#include <iro/include.hpp>
#include <iro/resources/resource.hpp>

class callbackRes : public resource
{
public:
    callbackRes(wl_client& client, unsigned int id);
};
