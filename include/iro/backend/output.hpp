#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>
#include <iro/util/global.hpp>

#include <nytl/rect.hpp>
#include <nytl/vec.hpp>
#include <nytl/time.hpp>
#include <nytl/callback.hpp>

#include <vector>

namespace ny
{
	//prototype
	class DrawContext;
}

namespace iro
{


//The output class represents a physical output on which the shell can
///render itself and the wayland surfaces.
///Abstract base class, backends should implement their output classes with
///backend-specific contexts. 
class Output : public Global
{
protected:
    unsigned int id_;
	nytl::vec2i position_;
	nytl::vec2ui size_;

    bool repaintScheduled_ = 0;
	wl_event_source* redrawEventSource_ {nullptr};
	nytl::timer lastRedraw_;

	std::unique_ptr<ny::DrawContext> drawContext_;
	nytl::callback<void(Output&, ny::DrawContext&)> drawCallback_;

	Compositor* compositor_;
	std::vector<SurfaceRes*> mappedSurfaces_;

public:
    Output(Compositor& comp, unsigned int id, const nytl::vec2i& pos = {0,0}, 
			const nytl::vec2ui& size = {0,0});
    virtual ~Output();

	///Schedules a repaint.
    void scheduleRepaint();

	///Returns if a repaint is currently scheduled.
    bool repaintScheduled() const { return repaintScheduled_; }

	///Returns the area in which the output is located in the internal coordinate system.
	nytl::rect2i extents() const;

	///Returns the position of the output in the internal coordinate system.
    const nytl::vec2i& position() const;

	///Returns the unique id of this output.
	unsigned int id() const { return id_; }

	///Returns the virtual (!) size this output has inside the internal coordinate system.
    const nytl::vec2ui& size() const;

	///Informs an output that a surface is now mapped in its area. Schedules a repaint if the
	///surface was not mapped on this output before.
	void mapSurface(SurfaceRes& surf);

	///Informs an output that a surface is no longer mapped in its area. Schedules a repaint if
	///the surface was mapped on this output before. Returns if the surface was mapped before.
	bool unmapSurface(SurfaceRes& surf);

	///Returns if a given surface is mapped on the output object.
	bool surfaceMapped(const SurfaceRes& surf) const;

	///Returns the first surface that is located at the given position. Returns nullptr
	///if none is located at this position.
	//todo: return all surfaces for this position not just a random one (vector)
    SurfaceRes* surfaceAt(const nytl::vec2i& pos) const;

	///Returns all surfaces that are mapped on this output.
	std::vector<SurfaceRes*> mappedSurfaces() const { return mappedSurfaces_; }

	///Returns the compositor this output is connected to.
	Compositor& compositor() const { return *compositor_; }

	///Registers a callback function that will be called when the output is redrawn.
	///The callback function must have a signature compatible to void(output&, ny::dc&)
	template<typename F> nytl::connection onDraw(F&& f){ return drawCallback_.add(f); }
	
	///Abstract base function for sending its own information to a wayland output
	///resource.
    virtual void sendInformation(const OutputRes& res) const = 0;

	///Redraws the output immidiatly. You usually don't want to call this directly, use 
	//scheduleRepaint instead.
	virtual void redraw();
};


///Represents a client resource for an output global.
class OutputRes : public Resource
{
protected:
    Output* output_;

public:
    OutputRes(Output& out, wl_client& client, unsigned int id, unsigned int version);
    Output& output() const { return *output_; }

    virtual unsigned int type() const override { return resourceType::output; }
};

}
