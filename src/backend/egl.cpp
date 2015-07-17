#include <backend/egl.hpp>

#include <util/misc.hpp>

#include <iostream>
#include <stdexcept>

PFNEGLBINDWAYLANDDISPLAYWL eglContext::eglBindWaylandDisplayWL = nullptr;
PFNEGLUNBINDWAYLANDDISPLAYWL eglContext::eglUnbindWaylandDisplayWL = nullptr;
PFNEGLQUERYWAYLANDBUFFERWL eglContext::eglQueryWaylandBufferWL = nullptr;
PFNEGLCREATEIMAGEKHRPROC eglContext::eglCreateImageKHR = nullptr;
PFNEGLDESTROYIMAGEKHRPROC eglContext::eglDestroyImageKHR = nullptr;

///////////////////////////////////////////////////////////
eglContext::eglContext(EGLNativeDisplayType display)
{
    display_ = eglGetDisplay(display);
    if(!display_)
    {
        throw std::runtime_error("could not get egl display");
        return;
    }

    EGLint major, minor;

    if(!eglInitialize(display_, &major, &minor))
    {
        throw std::runtime_error("could not initalize egl");
        return;
    }

    if(!eglBindAPI(EGL_OPENGL_ES_API))
    {
        throw std::runtime_error("could not bind egl opengl es api");
        return;
    }

    const EGLint attr[] =
    {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 1,
        EGL_GREEN_SIZE, 1,
        EGL_BLUE_SIZE, 1,
        EGL_ALPHA_SIZE, 0,
        EGL_DEPTH_SIZE, 0,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
        EGL_NONE
    };

    EGLint configCount;
    if(!eglChooseConfig(display_, attr, &config_, 1, &configCount))
    {
        throw std::runtime_error("could not get egl config");
        return;
    }

    const EGLint contextAttr[] =
    {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
    };

    context_ = eglCreateContext(display_, config_, EGL_NO_CONTEXT, contextAttr);
    if (!context_)
    {
        throw std::runtime_error("could not create egl config");
        return;
    }

    std::string ext = eglQueryString(display_, EGL_EXTENSIONS);
    extensions_ = split(ext, ' ');

    if(hasExtension("EGL_WL_bind_wayland_display") && hasExtension("EGL_KHR_image_base"))
    {
        eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress("eglCreateImageKHR");
        eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC) eglGetProcAddress("eglDestroyImageKHR");

        eglBindWaylandDisplayWL = (PFNEGLBINDWAYLANDDISPLAYWL) eglGetProcAddress("eglBindWaylandDisplayWL");
        eglUnbindWaylandDisplayWL = (PFNEGLUNBINDWAYLANDDISPLAYWL) eglGetProcAddress("eglUnbindWaylandDisplayWL");
        eglQueryWaylandBufferWL = (PFNEGLQUERYWAYLANDBUFFERWL) eglGetProcAddress("eglQueryWaylandBufferWL");

        if(!eglCreateImageKHR || !eglDestroyImageKHR || !eglBindWaylandDisplayWL || !eglUnbindWaylandDisplayWL || !eglQueryWaylandBufferWL)
        {
            throw std::runtime_error("could not load all needed egl extension functions");
            return;
        }
    }
    else
    {
        throw std::runtime_error("needed egl extensions extension not supported");
        return;
    }

    eglBindWaylandDisplayWL(display_, iroWlDisplay());
}

eglContext::~eglContext()
{
    //eglUnbindWaylandDisplayWL(display_, iroCompositor()->iroWlDisplay());

    eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(display_, context_);
    eglTerminate(display_);
}

bool eglContext::hasExtension(const std::string& extension) const
{
    for(size_t i(0); i < extensions_.size(); i++)
    {
        if(extensions_[i] == extension)
            return 1;
    }

    return 0;
}
