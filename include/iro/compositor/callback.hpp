#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>

namespace iro
{

class CallbackRes : public Resource
{
public:
    CallbackRes(wl_client& client, unsigned int id);

	virtual unsigned int type() const override { return resourceType::callback; }
};

}
