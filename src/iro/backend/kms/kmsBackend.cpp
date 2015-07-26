#include <iro/backend/kms/kmsBackend.hpp>
#include <iro/backend/kms/input.hpp>
#include <iro/compositor/compositor.hpp>
#include <iro/backend/tty.hpp>
#include <iro/backend/renderer.hpp>
#include <iro/backend/session.hpp>
#include <iro/backend/egl.hpp>

#include <nyutil/misc.hpp>

#include <iro/log.hpp>

#include <GL/gl.h>

#include <fcntl.h>
#include <xf86drm.h>
#include <stdlib.h>
#include <string.h>

#include <stdexcept>
#include <iostream>

kmsBackend* getKMSBackend()
{
    if(!iroBackend()) return nullptr;
    return dynamic_cast<kmsBackend*>(iroBackend());
}

ttyHandler* iroTTYHandler()
{
    if(!getKMSBackend()) return nullptr;
    return getKMSBackend()->getTTYHandler();
}

inputHandler* iroInputHandler()
{
    if(!getKMSBackend()) return nullptr;
    return getKMSBackend()->getInputHandler();
}

///////////////////////////////////////////////////////////////////////////
void drmPageFlipEvent(int fd, unsigned int frame, unsigned int sec, unsigned int usec, void *data)
{
    ((kmsOutput*)data)->wasFlipped();
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
    std::cout << "0" << std::endl;

    tty_ = new ttyHandler(*iroSessionManager());
    tty_->beforeEnter(memberCallback(&kmsBackend::onTTYEnter, this));
    tty_->beforeLeave(memberCallback(&kmsBackend::onTTYLeave, this));

    input_ = new inputHandler(*iroSessionManager());

    std::string path = "/dev/dri/card0";
    drm_ = iroSessionManager()->takeDevice(path);

    if(!drm_ || !drm_->fd)
    {
        throw std::runtime_error("kmsBackend::kmsBackend: cant take drm device at " + path);
        return;
    }
    drm_->onPause(memberCallback(&kmsBackend::onDRMPause, this));
    drm_->onResume(memberCallback(&kmsBackend::onDRMResume, this));

    gbmDevice_ = gbm_create_device(drm_->fd);
    if(!gbmDevice_)
    {
        throw std::runtime_error("kmsBackend::kmsBackend: cant create gbm device");
        return;
    }

    int i;
    drmModeRes* resources = drmModeGetResources(drm_->fd);
    if (!resources)
    {
        throw std::runtime_error("kmsBackend::kmsBackend: drmModeGetResources failed");
        return;
    }

    for (i = 0; i < resources->count_connectors; i++)
    {
        drmConnector_ = drmModeGetConnector(drm_->fd, resources->connectors[i]);
        if (!drmConnector_)
            continue;

        if (drmConnector_->connection == DRM_MODE_CONNECTED && drmConnector_->count_modes > 0)
            break;

        drmModeFreeConnector(drmConnector_);
    }

    if (i == resources->count_connectors)
    {
        throw std::runtime_error("kmsBackend::kmsBackend: no active connector found");
        return;
    }

    for (i = 0; i < resources->count_encoders; i++)
    {
        drmEncoder_ = drmModeGetEncoder(drm_->fd, resources->encoders[i]);

        if (!drmEncoder_)
            continue;

        if (drmEncoder_->encoder_id == drmConnector_->encoder_id)
            break;

        drmModeFreeEncoder(drmEncoder_);
    }

    drmMode_ = drmConnector_->modes[0];

    drmSavedCrtc_ = drmModeGetCrtc(drm_->fd, drmEncoder_->crtc_id);
    if(!drmSavedCrtc_)
    {
        throw std::runtime_error("kmsBackend::kmsBackend: cant save crtc");
        return;
    }

    drmEventSource_ = wl_event_loop_add_fd(iroWlEventLoop(), drm_->fd, WL_EVENT_READABLE, drmEvent, this);

    eglContext_ = new eglContext(gbmDevice_);

    const unsigned int numOutputs = 1; //TODO
    for(unsigned int i(0); i < numOutputs; i++)
    {
        kmsOutput* out = new kmsOutput(*this, 0);
        outputs_.push_back(out);
    }

    renderer_ = new renderer(); //must be initialized after, because it needs a valid eglContext (is made current by outoput)

    for(output* out : outputs_)
    {
        ((kmsOutput*)out)->setCrtc();
        out->refresh();
    }
}

kmsBackend::~kmsBackend()
{
    for(auto* out : outputs_)
        delete out;

    if(drmSavedCrtc_)drmModeSetCrtc(drm_->fd, drmSavedCrtc_->crtc_id, drmSavedCrtc_->buffer_id, drmSavedCrtc_->x, drmSavedCrtc_->y, &drmConnector_->connector_id, 1, &drmSavedCrtc_->mode);

    if(wlEventSource_) wl_event_source_remove(wlEventSource_);
    if(eglContext_) delete eglContext_;
    if(gbmDevice_) gbm_device_destroy(gbmDevice_);
    if(tty_) delete tty_;
    if(drm_) iroSessionManager()->releaseDevice(*drm_);
}

void kmsBackend::onTTYEnter()
{
    for(auto* out : outputs_)
    {
        kmsOutput* kout = (kmsOutput*) out;
        kout->setCrtc();
        kout->refresh();
    }
}

void kmsBackend::onTTYLeave()
{
    drmModeSetCrtc(drm_->fd, drmSavedCrtc_->crtc_id, drmSavedCrtc_->buffer_id, drmSavedCrtc_->x, drmSavedCrtc_->y, &drmConnector_->connector_id, 1, &drmSavedCrtc_->mode);
}

void kmsBackend::onDRMPause()
{

}

void kmsBackend::onDRMResume()
{

}

int kmsBackend::getFD() const
{
    return (drm_) ? drm_->fd : -1;
}

////////////////////////////////////////////////////////////////////////////////////
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
}

kmsOutput::~kmsOutput()
{
    releaseFB(fbs_[0]);
    releaseFB(fbs_[1]);

    gbm_surface_destroy(gbmSurface_);
}

void kmsOutput::render()
{
    if(flipping_)
        return;

    output::render();
}

void kmsOutput::setCrtc()
{
    set = 0;

    if(fbs_[frontBuffer_].valid())
    {
        drmModeSetCrtc(getKMSBackend()->getFD(), getKMSBackend()->getDRMEncoder()->crtc_id, fbs_[frontBuffer_].fb, 0, 0, &getKMSBackend()->getDRMConnector()->connector_id, 1, &getKMSBackend()->getDRMMode());
        set = 1;
    }
}

void kmsOutput::createFB(fb& obj)
{
    obj.buffer = gbm_surface_lock_front_buffer(gbmSurface_);

    unsigned int width = gbm_bo_get_width(obj.buffer);
    unsigned int height = gbm_bo_get_height(obj.buffer);
    unsigned int handle = gbm_bo_get_handle(obj.buffer).u32;
    unsigned int stride = gbm_bo_get_stride(obj.buffer);

    drmModeAddFB(getKMSBackend()->getFD(), width, height, 24, 32, stride, handle, &obj.fb);

    if(!set) drmModeSetCrtc(getKMSBackend()->getFD(), getKMSBackend()->getDRMEncoder()->crtc_id, obj.fb, 0, 0, &getKMSBackend()->getDRMConnector()->connector_id, 1, &getKMSBackend()->getDRMMode());
    set = 1;
}

void kmsOutput::releaseFB(fb& obj)
{
    if(obj.buffer)drmModeRmFB(getKMSBackend()->getFD(), obj.fb);
    if(obj.fb)gbm_surface_release_buffer(gbmSurface_, obj.buffer);

    obj.buffer = nullptr;
    obj.fb = 0;
}

void kmsOutput::swapBuffers()
{
    if(!flipping_ && getKMSBackend() && iroTTYHandler()->focus())
    {
        eglSwapBuffers(iroEglContext()->getDisplay(), eglSurface_);

        releaseFB(fbs_[frontBuffer_]);
        createFB(fbs_[frontBuffer_]);

        drmModePageFlip(getKMSBackend()->getFD(), getKMSBackend()->getDRMEncoder()->crtc_id, fbs_[frontBuffer_].fb, DRM_MODE_PAGE_FLIP_EVENT, this);

        flipping_ = 1;
    }
}

void kmsOutput::wasFlipped()
{
    frontBuffer_ ^= 1;
    flipping_ = 0;

    releaseFB(fbs_[frontBuffer_]);
}

vec2ui kmsOutput::getSize() const
{
    vec2ui ret;
    ret.x = getKMSBackend()->getDRMMode().vdisplay;
    ret.y = getKMSBackend()->getDRMMode().hdisplay;
    return ret;
}
