#pragma once

#include <iro/include.hpp>
#include <nyutil/nonCopyable.hpp>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <string>
#include <vector>

struct gbm_device;

class eglContext : public nonCopyable
{
protected:
    EGLDisplay display_;
    EGLContext context_;
    EGLConfig config_;

    std::vector<std::string> extensions_;

public:
    eglContext(void* display);
    virtual ~eglContext();

    EGLDisplay getDisplay() const { return display_; }
    EGLContext getContext() const { return context_; }
    EGLConfig getConfig() const { return config_; }

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
