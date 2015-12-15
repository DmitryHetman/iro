#include <iro/backend/backend.hpp>
#include <iro/backend/output.hpp>
#include <nytl/log.hpp>

namespace iro
{

Backend::Backend() = default;
Backend::~Backend()
{
	//before anything implicitly will be destructed
	nytl::sendLog("Finished ~Backend, ", outputs_.size(), " left");
	*nytl::sendLog.stream << std::endl;
	outputs_.clear(); 
}

Output* Backend::outputAt(const nytl::vec2i& pos) const
{
	for(auto& o : outputs_)
	{
		if(contains(o->extents(), pos))
			return o.get();
	}

	return nullptr;
}

std::vector<Output*> Backend::outputsAt(const nytl::rect2i& area) const
{
	std::vector<Output*> ret;
	for(auto& o : outputs_)
	{
		if(intersects(o->extents(), area))
			ret.push_back(o.get());
	}

	return ret;
}

std::vector<Output*> Backend::outputs() const
{
	std::vector<Output*> ret;
	for(auto& o : outputs_)
		ret.push_back(o.get());

	return ret;
}

std::vector<Output*> Backend::outputsAt(const nytl::region2i& area) const
{
	std::vector<Output*> ret;
	for(auto& o : outputs_)
	{
		if(intersects(o->extents(), area))
			ret.push_back(o.get());
	}

	return ret;
}

bool Backend::destroyOutput(const Output& op)
{
	for(auto it = outputs_.cbegin(); it != outputs_.cend(); ++it)
	{
		if(it->get() == &op)
		{
			outputDestroyedCallback_(*it->get());
			outputs_.erase(it);
			return 1;
		}
	}

	return  0;
}

void Backend::addOutput(std::unique_ptr<Output>&& op)
{
	outputCreatedCallback_(*op);
	outputs_.push_back(std::move(op));
}

}
