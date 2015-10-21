#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>

#include <nyutil/nonCopyable.hpp>
#include <nyutil/rect.hpp>
#include <nyutil/vec.hpp>
#include <nyutil/time.hpp>

#include <ny/surface.hpp>

#include <vector>

output* outputAt(int x, int y);
output* outputAt(vec2i pos);

std::vector<output*> outputsAt(int x, int y, int w, int h);
std::vector<output*> outputsAt(vec2i pos, vec2i size);
std::vector<output*> outputsAt(rect2i ext);

class output : public ny::surface
{

friend class renderer;
friend int outputRedraw(void*);

protected:
	unsigned int id_;
	vec2ui size_;
	vec2i position_;

	bool repaintScheduled_ = 0;
	wl_event_source* drawEventSource_ = nullptr;
	timer lastRedraw_;

	std::vector<surfaceRes*> mappedSurfaces_; //no need for ref here, surfaces unmap theirself at destruction
	wl_global* global_ = nullptr;
	renderData* renderData_ = nullptr;

	virtual void render();

public:
    output(unsigned int id);
    virtual ~output();

	void scheduleRepaint();

	void mapSurface(surfaceRes& surf);
	void unmapSurface(surfaceRes& surf);

	rect2i getExtents() const;
	vec2i getPosition() const;
	virtual vec2ui getSize() const override; //ny::surface

	surfaceRes* getSurfaceAt(const vec2i& pos);

	virtual void* getNativeSurface() const = 0;
	virtual void sendInformation(const outputRes& res) const = 0;
};

//////////////////
class outputRes : public resource
{
protected:
    output& output_;

public:
    outputRes(output& out, wl_client& client, unsigned int id, unsigned int version);
    output& getOutput() const { return output_; }

    //res
    virtual resourceType getType() const override { return resourceType::output; }
};
