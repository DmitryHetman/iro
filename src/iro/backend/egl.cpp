#include <iro/backend/egl.hpp>
#include <iro/backend/output.hpp>

#include <nyutil/misc.hpp>

#include <iostream>
#include <stdexcept>

PFNEGLBINDWAYLANDDISPLAYWL eglContext::eglBindWaylandDisplayWL = nullptr;
PFNEGLUNBINDWAYLANDDISPLAYWL eglContext::eglUnbindWaylandDisplayWL = nullptr;
PFNEGLQUERYWAYLANDBUFFERWL eglContext::eglQueryWaylandBufferWL = nullptr;
PFNEGLCREATEIMAGEKHRPROC eglContext::eglCreateImageKHR = nullptr;
PFNEGLDESTROYIMAGEKHRPROC eglContext::eglDestroyImageKHR = nullptr;

///////////////////////////////////////////////////////////
eglContext::eglContext(void* display) : ny::eglAppContext()
{
    eglDisplay_ = eglGetDisplay((EGLNativeDisplayType)display);
    if(!eglDisplay_)
    {
        throw std::runtime_error("could not get egl display");
        return;
    }

    EGLint major, minor;

    if(!eglInitialize(eglDisplay_, &major, &minor))
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
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

    EGLint configCount;
    if(!eglChooseConfig(eglDisplay_, attr, &eglConfig_, 1, &configCount))
    {
        throw std::runtime_error("could not get egl config");
        return;
    }

    const EGLint contextAttr[] =
    {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
    };

    eglContext_ = eglCreateContext(eglDisplay_, eglConfig_, EGL_NO_CONTEXT, contextAttr);
    if (!eglContext_)
    {
        throw std::runtime_error("could not create egl config");
        return;
    }

    std::string ext = eglQueryString(eglDisplay_, EGL_EXTENSIONS);
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

    eglBindWaylandDisplayWL(eglDisplay_, iroWlDisplay());
}

eglContext::~eglContext()
{
    //eglUnbindWaylandDisplayWL(display_, iroCompositor()->iroWlDisplay());

    eglMakeCurrent(eglDisplay_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(eglDisplay_, eglContext_);
    eglTerminate(eglDisplay_);
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

bool eglContext::makeCurrent(EGLSurface surf)
{
    if(eglContext_ && eglDisplay_ && surf) return eglMakeCurrent(eglDisplay_, surf, surf, eglContext_);
    else return 0;
}

bool eglContext::makeNotCurrent()
{
    if(eglDisplay_) return eglMakeCurrent(eglDisplay_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    else return 0;
}

bool eglContext::isCurrent()
{
    return (eglGetCurrentContext() == eglContext_);
}
