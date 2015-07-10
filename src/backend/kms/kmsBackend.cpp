#include <backend/kms/kmsBackend.hpp>
#include <backend/kms/input.hpp>
#include <compositor/compositor.hpp>
#include <backend/egl.hpp>
#include <backend/tty.hpp>
#include <backend/renderer.hpp>

#include <util/misc.hpp>

#include <GL/gl.h>

#include <fcntl.h>
#include <xf86drm.h>
#include <stdlib.h>
#include <string.h>

#include <stdexcept>
#include <iostream>

kmsBackend* getKMSBackend()
{
    if(!getBackend()) return nullptr;
    return dynamic_cast<kmsBackend*>(getBackend());
}

ttyHandler* getTTYHandler()
{
    if(!getKMSBackend()) return nullptr;
    return getKMSBackend()->getTTYHandler();
}

inputHandler* getInputHandler()
{
    if(!getKMSBackend()) return nullptr;
    return getKMSBackend()->getInputHandler();
}

///////////////////////////////////////////////////////////////////////////
void drmPageFlipEvent(int fd, unsigned int frame, unsigned int sec, unsigned int usec, void *data)
{
    ((kmsOutput*)getKMSBackend()->getOutput())->wasFlipped();
}

int drmEvent(int fd, unsigned int mask, void* data)
{
    drmEventContext ev;
    memset(&ev, 0, sizeof(ev));
    ev.version = DRM_EVENT_CONTEXT_VERSION;
    ev.page_flip_handler = drmPageFlipEvent;
    drmHandleEvent(fd, &ev);

    return 0;
}

///////////////////////////////////////////////////////////////////////////
kmsBackend::kmsBackend()
{
    tty_ = new ttyHandler();
    tty_->beforeEnter(memberCallback(&kmsBackend::onEnter, this));
    tty_->beforeLeave(memberCallback(&kmsBackend::onLeave, this));

    input_ = new inputHandler();

    fd_ = open("/dev/dri/card0", O_RDWR);
    if(!fd_)
    {
        throw std::runtime_error("cant open device");
        return;
    }

    gbmDevice_ = gbm_create_device(fd_);
    if(!gbmDevice_)
    {
        throw std::runtime_error("cant create gbm device");
        return;
    }

    int i;
    drmModeRes* resources = drmModeGetResources(fd_);
    if (!resources)
    {
        throw std::runtime_error("drmModeGetResources failed");
        return;
    }

    for (i = 0; i < resources->count_connectors; i++)
    {
        drmConnector_ = drmModeGetConnector(fd_, resources->connectors[i]);
        if (!drmConnector_)
            continue;

        if (drmConnector_->connection == DRM_MODE_CONNECTED && drmConnector_->count_modes > 0)
            break;

        drmModeFreeConnector(drmConnector_);
    }

    if (i == resources->count_connectors)
    {
        throw std::runtime_error("no active connector found");
        return;
    }

    for (i = 0; i < resources->count_encoders; i++)
    {
        drmEncoder_ = drmModeGetEncoder(fd_, resources->encoders[i]);

        if (!drmEncoder_)
            continue;

        if (drmEncoder_->encoder_id == drmConnector_->encoder_id)
            break;

        drmModeFreeEncoder(drmEncoder_);
    }

    drmMode_ = drmConnector_->modes[0];

    drmSavedCrtc_ = drmModeGetCrtc(fd_, drmEncoder_->crtc_id);
    if(!drmSavedCrtc_)
    {
        throw std::runtime_error("cant save crtc");
        return;
    }

    drmEventSource_ = wl_event_loop_add_fd(getWlEventLoop(), fd_, WL_EVENT_READABLE, drmEvent, this);

    eglContext_ = new eglContext((EGLNativeDisplayType)gbmDevice_);
    outputs_.push_back(new kmsOutput(*this, 0));

    onEnter();
}

kmsBackend::~kmsBackend()
{
    delete outputs_[0];
    outputs_.erase(outputs_.begin());


    drmModeSetCrtc(fd_, drmSavedCrtc_->crtc_id, drmSavedCrtc_->buffer_id, drmSavedCrtc_->x, drmSavedCrtc_->y, &drmConnector_->connector_id, 1, &drmSavedCrtc_->mode);

    if(eglContext_) delete eglContext_;
    if(tty_) delete tty_;

    if(gbmDevice_)gbm_device_destroy(gbmDevice_);
}

void kmsBackend::onEnter()
{
    kmsOutput* out = (kmsOutput*) outputs_[0];
    drmModeSetCrtc(fd_, drmEncoder_->crtc_id, out->getFBID(), 0, 0, &drmConnector_->connector_id, 1, &drmMode_);
}

void kmsBackend::onLeave()
{
    drmModeSetCrtc(fd_, drmSavedCrtc_->crtc_id, drmSavedCrtc_->buffer_id, drmSavedCrtc_->x, drmSavedCrtc_->y, &drmConnector_->connector_id, 1, &drmSavedCrtc_->mode);
}

////////////////////77
kmsOutput::kmsOutput(const kmsBackend& kms, unsigned int id) : output(id)
{
    eglContext* egl = kms.getEglContext();

    unsigned int height = kms.getDRMMode().vdisplay;
    unsigned int width = kms.getDRMMode().hdisplay;

    gbmSurface_ = gbm_surface_create(kms.getGBMDevice(), width, height, GBM_BO_FORMAT_XRGB8888, GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
    if(!gbmSurface_)
    {
        throw std::runtime_error("cant create gbmSurface on kms output");
        return;
    }

    eglSurface_ = eglCreateWindowSurface(egl->getDisplay(), egl->getConfig(), (EGLNativeWindowType) gbmSurface_, nullptr);
    if(!gbmSurface_)
    {
        throw std::runtime_error("cant create eglSurface ons kms output");
        return;
    }

    if(!eglMakeCurrent(egl->getDisplay(), eglSurface_, eglSurface_, egl->getContext()))
    {
        throw std::runtime_error("cant make egl Context current on kms output");
        return;
    }

    ////////////////////////////
    glViewport(0, 0, (GLint) width, (GLint) height);
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glFinish();
    ////////////////////////////

    eglSwapBuffers(egl->getDisplay(), eglSurface_);

    gbmBuffer_ = gbm_surface_lock_front_buffer(gbmSurface_);
    drmModeAddFB(kms.getFD(), width, height, 24, 32, gbm_bo_get_stride(gbmBuffer_), gbm_bo_get_handle(gbmBuffer_).u32, &fbID_);

    //drmModePageFlip(kms.getFD(), kms.getDRMEncoder()->crtc_id, fbID_, DRM_MODE_PAGE_FLIP_EVENT, this);

    renderer_ = new renderer();
}

kmsOutput::~kmsOutput()
{
    drmModeRmFB(getKMSBackend()->getFD(), fbID_);
}

void kmsOutput::swapBuffers()
{
    if(!flipping_ && getKMSBackend() && getTTYHandler()->focus())
    {
        eglSwapBuffers(getEglContext()->getDisplay(), eglSurface_);

        unsigned int height = getKMSBackend()->getDRMMode().vdisplay;
        unsigned int width = getKMSBackend()->getDRMMode().hdisplay;

        gbm_bo* oldBuffer = gbmBuffer_;
        int oldFB = fbID_;

        gbmBuffer_ = gbm_surface_lock_front_buffer(gbmSurface_);
        drmModeAddFB(getKMSBackend()->getFD(), width, height, 24, 32, gbm_bo_get_stride(gbmBuffer_), gbm_bo_get_handle(gbmBuffer_).u32, &fbID_);

        drmModePageFlip(getKMSBackend()->getFD(), getKMSBackend()->getDRMEncoder()->crtc_id, fbID_, DRM_MODE_PAGE_FLIP_EVENT, this);

        if(oldBuffer) gbm_surface_release_buffer(gbmSurface_, oldBuffer);
        if(oldFB) drmModeRmFB(getKMSBackend()->getFD(), oldFB);

        flipping_ = 1;
    }

}

void kmsOutput::makeEglCurrent()
{
    eglMakeCurrent(getEglContext()->getDisplay(), eglSurface_, eglSurface_, getEglContext()->getContext());
}

vec2ui kmsOutput::getSize() const
{
    vec2ui ret;
    ret.x = getKMSBackend()->getDRMMode().vdisplay;
    ret.y = getKMSBackend()->getDRMMode().hdisplay;
    return ret;
}
