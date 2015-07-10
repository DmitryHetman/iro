#pragma once

#include <iro.hpp>
#include <resources/resource.hpp>

class callbackRes : public resource
{
public:
    callbackRes(wl_client* client, unsigned int id);
};
