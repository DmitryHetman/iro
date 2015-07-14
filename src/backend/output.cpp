#include <backend/output.hpp>

#include <backend/egl.hpp>
#include <backend/renderer.hpp>
#include <resources/surface.hpp>
#include <seat/seat.hpp>

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <wayland-server-protocol.h>

int outputRedraw(void* data)
{
    output* o = (output*) data;
    o->render();

    return 1;
}

//////////////////////////////
void bindOutput(wl_client* client, void* data, unsigned int version, unsigned int id)
{
    output* out = (output*) data;
    new outputRes(out, client, version, id);
}

//////////////////////////
output::output(unsigned int id) : id_(id)
{
    global_ = wl_global_create(getWlDisplay(), &wl_output_interface, 2, this, bindOutput);

    drawEventSource_ = wl_event_loop_add_timer(getWlEventLoop(), outputRedraw, this);
}

output::~output()
{
    wl_event_source_remove(drawEventSource_);
    if(renderer_) delete renderer_;

    wl_global_destroy(global_);
}

void output::render()
{
    makeEglCurrent();

    glClearColor(0.8, 0.8, 0.3, 1);
    glClear(GL_COLOR_BUFFER_BIT);


    for(unsigned int i(0); i < mappedSurfaces_.size(); i++)
    {
        renderer_->render(mappedSurfaces_[i]);
    }

    renderer_->drawCursor(getSeat()->getPointer());

    glFinish();
    swapBuffers();
}

void output::refresh()
{
    wl_event_source_timer_update(drawEventSource_, 1);
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


///////////////////////7
outputRes::outputRes(output* out, wl_client* client, unsigned int id, unsigned int version) : resource(client, id, &wl_output_interface, nullptr, version), output_(out)
{

}
