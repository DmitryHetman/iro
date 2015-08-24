#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>

#include <nyutil/nonCopyable.hpp>
#include <nyutil/rect.hpp>
#include <nyutil/vec.hpp>

#include <vector>

output* outputAt(int x, int y);
output* outputAt(vec2i pos);

std::vector<output*> outputsAt(int x, int y, int w, int h);
std::vector<output*> outputsAt(vec2i pos, vec2i size);
std::vector<output*> outputsAt(rect2i ext);

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
};

//////////////////
class outputRes : public resource
{
protected:
    output& output_;

public:
    outputRes(output& out, wl_client& client, unsigned int id, unsigned int version);

    output& getOutput() const { return output_; }
};
