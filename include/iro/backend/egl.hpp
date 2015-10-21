#pragma once

#include <iro/include.hpp>
#include <ny/gl/egl.hpp>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <string>
#include <vector>

struct gbm_device;

class eglContext : public ny::eglAppContext
{
protected:
    EGLContext eglContext_; //just one context for all surfaces
    std::vector<std::string> extensions_;

public:
    eglContext(void* display);
    virtual ~eglContext();

    EGLContext getContext() const { return eglContext_; }

    bool makeCurrent(EGLSurface surf);
    bool makeNotCurrent();
    bool isCurrent();

    bool hasExtension(const std::string& extension) const;

    static PFNEGLBINDWAYLANDDISPLAYWL eglBindWaylandDisplayWL;
    static PFNEGLUNBINDWAYLANDDISPLAYWL eglUnbindWaylandDisplayWL;
    static PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL;
    static PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR;
    static PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR;
};
