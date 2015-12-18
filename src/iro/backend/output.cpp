#include <iro/backend/output.hpp>
#include <iro/compositor/resource.hpp>
#include <iro/compositor/client.hpp>
#include <iro/compositor/compositor.hpp>
#include <iro/compositor/surface.hpp>

#include <ny/draw/drawContext.hpp>

#include <nytl/make_unique.hpp>
#include <nytl/log.hpp>

#include <wayland-server-protocol.h>

namespace iro
{

OutputRes::OutputRes(Output& out, wl_client& client, unsigned int id, unsigned int version)
	: Resource(client, id, &wl_output_interface, nullptr, version), output_(&out) {}

//callback
void bindOutput(wl_client* client, void* data, unsigned int version, unsigned int id)
{
    Output* out = static_cast<Output*>(data);
	if(!out)
	{
		nytl::sendWarning("bindOutput: invalid data");
		return;
	}

	auto& c = out->compositor().client(*client);
	auto outres = nytl::make_unique<OutputRes>(*out, *client, id, version);
	out->sendInformation(*outres);

	c.addResource(std::move(outres));
}

int outputRedraw(void* data)
{
    Output* o = static_cast<Output*>(data);
	if(!o)
	{
		nytl::sendWarning("outputRedraw: invalid data");
		return 1;
	}

    o->redraw();
    return 1;
}

//output implementation
Output::Output(Compositor& comp, unsigned int id, const nytl::vec2i& position, 
		const nytl::vec2ui& size) 
	: id_(id), position_(position), size_(size), compositor_(&comp)
{
    wlGlobal_ = wl_global_create(&comp.wlDisplay(), &wl_output_interface, 2, this, bindOutput);
    redrawEventSource_ = wl_event_loop_add_timer(&comp.wlEventLoop(), outputRedraw, this);
}

Output::~Output()
{
    wl_event_source_remove(redrawEventSource_);
}

void Output::redraw()
{
	repaintScheduled_ = 0;
	lastRedraw_.reset();

	drawCallback_(*this, *drawContext_);
}

void Output::scheduleRepaint()
{
	static const unsigned int maxFPS = 60; //todo: general?
	if(!repaintScheduled_)
	{
		//int time = (1000 / maxFPS) - lastRedraw_.getElapsedTime().asMilliseconds();
		//if(time < 0) time = 1;

		wl_event_source_timer_update(redrawEventSource_, 20);
		repaintScheduled_ = 1;
	}
}

void Output::mapSurface(SurfaceRes& surf)
{
	if(surfaceMapped(surf)) return;

    mappedSurfaces_.push_back(&surf);
    scheduleRepaint();
}

bool Output::surfaceMapped(const SurfaceRes& surf) const
{
	for(auto& s : mappedSurfaces_)
		if(s == &surf) return 1;

	return 0;
}

bool Output::unmapSurface(SurfaceRes& surf)
{
    for(unsigned int i(0); i < mappedSurfaces_.size(); i++)
    {
        if(mappedSurfaces_[i] == &surf)
		{
			mappedSurfaces_.erase(mappedSurfaces_.begin() + i);
			scheduleRepaint();
			return 1;
		}
    }

	return 0;
}

SurfaceRes* Output::surfaceAt(const nytl::vec2i& pos) const
{
    for(auto& res : mappedSurfaces_)
    {
        if(contains(res->extents(), pos))
            return res;
    }

    return nullptr;
}

const nytl::vec2i& Output::position() const
{
    return position_;
}

const nytl::vec2ui& Output::size() const
{
    return size_;
}

nytl::rect2i Output::extents() const
{
    return nytl::rect2i(position(), size());
}

}
