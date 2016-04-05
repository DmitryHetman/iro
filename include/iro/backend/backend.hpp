#pragma once

#include <iro/include.hpp>
#include <nytl/nonCopyable.hpp>
#include <nytl/callback.hpp>
#include <nytl/rectRegion.hpp>

#include <vector>
#include <memory>

namespace iro
{

///The backend class is the abstract base class representing a compositor backend.
///It is respsonsible for creating output and renderer as well as managing the internal
///coordinate system.
class Backend : public nytl::NonCopyable
{
protected:
    std::vector<std::unique_ptr<Output>> outputs_;

	nytl::Callback<void(Output&)> outputCreatedCallback_;
	nytl::Callback<void(Output&)> outputDestroyedCallback_;

	bool destroyOutput(const Output& op);
	void addOutput(std::unique_ptr<Output>&& op);

public:
    Backend();
    virtual ~Backend();

	///Returns the output at the given position.
	//todo: return all outputs for position? multiple for one pos allowed?
    Output* outputAt(const nytl::Vec2i& pos) const;

	///Returns all outputs that lay in the specified area.
    std::vector<Output*> outputsAt(const nytl::Rect2i& area) const;

	///Returns all outputs that lay in the specified area.
    std::vector<Output*> outputsAt(const nytl::RectRegion2i& area) const;

	///Returns all outputs that are owned by this backend.
    std::vector<Output*> outputs() const;

	///Registers a callback function that should be called when an output is created.
    template<typename F> nytl::Connection onOutputCreated(F&& f)
		{ return outputCreatedCallback_.add(f); }

	///Registers a callback function that should be called weh nan output is destroyed.
    template<typename F> nytl::Connection onOutputDestroyed(F&& f)
		{ return outputDestroyedCallback_.add(f); }

	///Creates a backend and renderer specific surface context.
	virtual std::unique_ptr<SurfaceContext> createSurfaceContext() const = 0;
	virtual WaylandEglContext* eglContext() const { return nullptr; }
};

}
