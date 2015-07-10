#include <backend/output.hpp>

#include <backend/egl.hpp>
#include <backend/renderer.hpp>
#include <compositor/compositor.hpp>
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
output::output()
{
    wl_global_create(getCompositor()->getWlDisplay(), &wl_output_interface, 2, this, bindOutput);
    drawEventSource_ = wl_event_loop_add_timer(getCompositor()->getWlEventLoop(), outputRedraw, this);
}

output::~output()
{
    if(renderer_) delete renderer_;
}

void output::render()
{
    makeEglCurrent();

    glClearColor(0.8, 0.8, 0.3, 1);
    glClear(GL_COLOR_BUFFER_BIT);


    for(unsigned int i(0); i < surfaces_.size(); i++)
    {
        renderer_->render(surfaces_[i]);
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
    surfaces_.push_back(surf);
    refresh();
}

void output::unmapSurface(surfaceRes* surf)
{
    for(unsigned int i(0); i < surfaces_.size(); i++)
    {
        if(surfaces_[i] == surf)
            surfaces_.erase(surfaces_.begin() + i);
    }
    refresh();
}

///////////////////////7
outputRes::outputRes(output* out, wl_client* client, unsigned int id, unsigned int version) : resource(client, id, &wl_output_interface, nullptr, version), output_(out)
{

}
