#include "x11Backend.hpp"
#include "server.hpp"
#include "types.hpp"

#include <wayland-server-protocol.h>

x11Backend::x11Backend()
{

}

bool x11Backend::init(vec2ui size)
{
    size_ = size;

    if(!(xDisplay_ = XOpenDisplay(NULL)))
    {
        return 0;
    }

    eventSource_ = wl_event_loop_add_fd(getServer()->getWlEventLoop(), ConnectionNumber(xDisplay_), WL_EVENT_READABLE, x11EventLoop, this);
    wl_event_source_check(eventSource_);

    GLint attribs[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    xVinfo_ = glXChooseVisual(xDisplay_, 0, attribs);
    if(!xVinfo_)
    {
        return 0;
    }

    XSetWindowAttributes attr;
    attr.colormap = XCreateColormap(xDisplay_, DefaultRootWindow(xDisplay_), xVinfo_->visual, AllocNone);
    attr.event_mask = ExposureMask | StructureNotifyMask;

    xWindow_ = XCreateWindow(xDisplay_, DefaultRootWindow(xDisplay_), 0, 0, size_.x, size_.y, 1, xVinfo_->depth, InputOutput, xVinfo_->visual, CWEventMask | CWColormap , &attr);

    XSizeHints s;
    s.min_width = size_.x; s.min_height = size_.y;
    s.min_width = size_.x; s.max_height = size_.y;
    s.flags |= PMinSize | PMaxSize;
    XSetWMNormalHints(xDisplay_, xWindow_, &s);

    glContext_ = glXCreateContext(xDisplay_, xVinfo_, NULL, GL_TRUE);
    glXMakeCurrent(xDisplay_, xWindow_, glContext_);
    glViewport(0, 0, size_.x, size_.y);
    glXSwapBuffers(xDisplay_, xWindow_);

    //todo: set Window close wm protocol

    XMapWindow(xDisplay_, xWindow_);
    XFlush(xDisplay_);

    return 1;
}

int x11Backend::eventLoop(int fd, unsigned int mask)
{
    int count = 0;

    while(XPending(xDisplay_) != 0)
    {
        XEvent ev;
        XNextEvent(xDisplay_, &ev);

        switch (ev.type)
        {

        case Expose:
        {
            //redraw();
        }

        default:
        {
            break;
        }

        }
    }

    return count;
}

void x11Backend::renderSurface(surface* s)
{
    /*
    wl_shm_buffer* shmBuffer = static_cast<wl_shm_buffer*>(s->getCurrent().buffer->data);
    unsigned int shmFormat = wl_shm_buffer_get_format(shmBuffer);

    void* data = wl_shm_buffer_get_data(shmBuffer);
    vec2i size(wl_shm_buffer_get_width(shmBuffer), wl_shm_buffer_get_height(shmBuffer));


    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data);


    glXSwapBuffers(xDisplay_, xWindow_);
    */
}

////////////////////////7
int x11EventLoop(int fd, unsigned int mask, void* data)
{
    x11Backend* b = static_cast<x11Backend*>(data);
    return b->eventLoop(fd, mask);
}
