#include <iro/backend/output.hpp>
#include <iro/backend/backend.hpp>

#include <iro/backend/egl.hpp>
#include <iro/backend/renderer.hpp>
#include <iro/compositor/surface.hpp>
#include <iro/compositor/shell.hpp>
#include <iro/seat/seat.hpp>

#include <iro/util/log.hpp>
#include <iro/util/iroModule.hpp>

#include <nyutil/time.hpp>

#include <wayland-server-protocol.h>

int outputRedraw(void* data)
{
    output* o = (output*) data;
    o->render();

    return 1;
}

//
output* outputAt(int x, int y)
{
    return outputAt(vec2i(x,y));
}

output* outputAt(vec2i pos)
{
    for(auto* out : iroBackend()->getOutputs())
    {
        if(out->getExtents().contains(pos))
            return out;
    }

    return nullptr;
}

//
std::vector<output*> outputsAt(int x, int y, int w, int h)
{
    return outputsAt(rect2i(x,y,w,h));
}

std::vector<output*> outputsAt(vec2i pos, vec2i size)
{
    return outputsAt(rect2i(pos, size));
}

std::vector<output*> outputsAt(rect2i ext)
{
    std::vector<output*> ret;

    for(auto* out : iroBackend()->getOutputs())
    {
        if(out->getExtents().intersects(ext))
            ret.push_back(out);
    }

    return ret;
}

//////////////////////////////
void bindOutput(wl_client* client, void* data, unsigned int version, unsigned int id)
{
    output* out = (output*) data;
    new outputRes(*out, *client, id, version);
}

//////////////////////////
output::output(unsigned int id) : id_(id)
{
    global_ = wl_global_create(iroWlDisplay(), &wl_output_interface, 2, this, bindOutput);
    drawEventSource_ = wl_event_loop_add_timer(iroWlEventLoop(), outputRedraw, this);
}

output::~output()
{
    wl_event_source_remove(drawEventSource_);
    wl_global_destroy(global_);
}

void output::render()
{
    renderer* machine = iroBackend()->getRenderer();
    if(!machine)
    {
        iroWarning("output::render: no valid renderer");
        return;
    }

    //timer t;

    machine->beginDraw(*this);

    iroShell()->getModule()->render(machine);
    for(auto* surf : mappedSurfaces_)
    {
        if(!iroBackend()->getRenderer()->render(*surf))
            iroWarning("output::render: failed to render mapped surface ", surf);
    }

    if(!iroBackend()->getRenderer()->drawCursor(*iroSeat()->getPointer()))
        iroWarning("output::render: failed to draw cursor");

    machine->endDraw(*this);

    //iroLog("output::render: frametime ", t.getElapsedTime().asMilliseconds()," ms");
    for(auto* surf : mappedSurfaces_) surf->frameDone();
}

void output::refresh()
{
    wl_event_source_timer_update(drawEventSource_, 5);
}

void output::mapSurface(surfaceRes* surf)
{
    mappedSurfaces_.push_back(surf);
    refresh();
}

void output::unmapSurface(surfaceRes* surf)
{
    for(unsigned int i(0); i < mappedSurfaces_.size(); i++)
    {
        if(mappedSurfaces_[i] == surf)
            mappedSurfaces_.erase(mappedSurfaces_.begin() + i);
    }
    refresh();
}

surfaceRes* output::getSurfaceAt(vec2i pos)
{
    for(surfaceRes* res : mappedSurfaces_)
    {
        if(res->getExtents().contains(pos))
            return res;
    }
    return nullptr;
}

void output::swapBuffers()
{
    if(getEglSurface() && iroEglContext())
        eglSwapBuffers(iroEglContext()->getDisplay(), getEglSurface());
}

vec2i output::getPosition() const
{
    return position_;
}

///////////////////////7
outputRes::outputRes(output& out, wl_client& client, unsigned int id, unsigned int version) : resource(client, id, &wl_output_interface, nullptr, version), output_(out)
{

}