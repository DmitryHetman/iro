#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>
#include <nytl/vec.hpp>

namespace iro
{

//bufferRes
class BufferRes : public Resource
{
public:
    BufferRes(wl_resource& res);
	virtual ~BufferRes();

	void sendRelease() const;

	virtual unsigned int type() const override { return resourceType::buffer; }
};

}
