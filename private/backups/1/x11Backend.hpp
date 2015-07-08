#pragma once

#include <utils/vec.hpp>

#include <ny/app.hpp>
#include <ny/window.hpp>

#include <GL/glx.h>
using namespace utils;

class server;
class wl_event_source;
class surface;

class x11Backend
{
protected:
    vec2ui size_ = vec2ui();

    Display* xDisplay_ = 0;
    Window xWindow_ = 0;

    XVisualInfo* xVinfo_ = 0;

    wl_event_source* eventSource_ = 0;

    GLXContext glContext_;

    friend int x11EventLoop(int fd, unsigned int mask, void* data);
    int eventLoop(int fd, unsigned int mask);

public:
    x11Backend();
    bool init(vec2ui size = vec2ui(800, 500));

    void renderSurface(surface* s);
};

int x11EventLoop(int fd, unsigned int mask, void* data);
