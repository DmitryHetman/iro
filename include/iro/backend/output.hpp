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

<<<<<<< HEAD
class output
{
protected:
	unsigned int id_;
	vec2ui size_;
	vec2i position_;

	bool repaintSchedules_ = 0;
	bool repaintNeeded_ = 0;
	wl_event_source* refreshTimer_ = nullptr;

	wl_global* global_ = nullptr;

	rendererOutputData rendererData_ = nullptr;

public:
	void scheduleRepaint();

	rect2i getExtents() const;
	vec2i getPosition() const;
	vec2ui getSize() const;
=======
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

	std::vector<surfaceRef> mappedSurfaces_;
	wl_global* global_ = nullptr;
	renderData* renderData_ = nullptr;

	void render();

	output(unsigned int id);
	virtual ~output();

public:
	void scheduleRepaint();

	void mapSurface(surfaceRes& surf);
	void unmapSurface(surfaceRes& surf);

	rect2i getExtents() const;
	vec2i getPosition() const;
	virtual vec2ui getSize() const override; //ny::surface

	virtual void* getNativeSurface() const = 0;
	virtual void sendInformation(const outputRes& res) const = 0;
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527
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
