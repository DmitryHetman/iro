#include <iro/backend/kmsBackend.hpp>
#include <iro/backend/input.hpp>
#include <iro/compositor/compositor.hpp>

#include <iro/backend/glRenderer.hpp>
#include <iro/backend/session.hpp>
#include <iro/backend/egl.hpp>
#include <iro/seat/pointer.hpp>
#include <iro/seat/keyboard.hpp>
#include <iro/seat/event.hpp>
#include <iro/util/log.hpp>
#include <nyutil/misc.hpp>

#include <wayland-server-protocol.h>

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

//drm/////////////////////////////////////////////////////////////////////////
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

    drmModeRes* resources = drmModeGetResources(drm_->fd);
    if (!resources)
    {
        throw std::runtime_error("kmsBackend::kmsBackend: drmModeGetResources failed");
        return;
    }

    //find outputs, todo: logging
    unsigned int c, e; //connector, encoder
    drmModeConnector* connector = nullptr;
    drmModeEncoder* encoder = nullptr;

    for (c = 0; c < (unsigned int) resources->count_connectors; c++)
    {
        if(!(connector = drmModeGetConnector(drm_->fd, resources->connectors[c])))
            continue;

        if (connector->connection == DRM_MODE_CONNECTED && connector->count_modes > 0) //valid connector found
        {
            for (e = 0; e < (unsigned int) resources->count_encoders; e++)
            {
                if(!(encoder = drmModeGetEncoder(drm_->fd, resources->encoders[e])))
                    continue;

                if (encoder->encoder_id == connector->encoder_id) //valid encoder found
                {
                    kmsOutput* out = new kmsOutput(*this, connector, encoder, outputs_.size());
                    outputs_.push_back(out);
                }

                drmModeFreeEncoder(encoder);
            }
        }

        drmModeFreeConnector(connector);
    }

    if (c == (unsigned int) resources->count_connectors)
    {
        throw std::runtime_error("kmsBackend::kmsBackend: no active connector found");
        return;
    }

    iroLog("kmsBackend: ", c, " valid connectors found, ", e, " valid encoders found, ", outputs_.size(), " outputs created.");
    drmEventSource_ = wl_event_loop_add_fd(iroWlEventLoop(), drm_->fd, WL_EVENT_READABLE, drmEvent, this);

    //input
    input_ = new inputHandler();
    //input_ = libinput_udev_create_context(&libinputImplementation, iroSessionManager(), iroSessionManager()->getUDev());
    //libinput_udev_assign_seat(input_, iroSessionManager()->getSeat().c_str());
    //inputEventSource_ = wl_event_loop_add_fd(iroWlEventLoop(), libinput_get_fd(input_), WL_EVENT_READABLE, inputEventLoop, this);

/*
    drmMode_ = drmConnector_->modes[0];
    drmSavedCrtc_ = drmModeGetCrtc(drm_->fd, drmEncoder_->crtc_id);
    if(!drmSavedCrtc_)
    {
        throw std::runtime_error("kmsBackend::kmsBackend: cant save crtc");
        return;
    }
*/
}

kmsBackend::~kmsBackend()
{
    if(drmEventSource_) wl_event_source_remove(drmEventSource_);

    if(gbmDevice_) gbm_device_destroy(gbmDevice_);
    if(drm_) iroSessionManager()->releaseDevice(*drm_);

    if(input_) delete input_;
}

void kmsBackend::onTTYEnter()
{

}

void kmsBackend::onTTYLeave()
{
    //drmModeSetCrtc(drm_->fd, drmSavedCrtc_->crtc_id, drmSavedCrtc_->buffer_id, drmSavedCrtc_->x, drmSavedCrtc_->y, &drmConnector_->connector_id, 1, &drmSavedCrtc_->mode);
}

void kmsBackend::onDRMPause()
{
    drmDropMaster(drm_->fd);
}

void kmsBackend::onDRMResume()
{
    drmSetMaster(drm_->fd);
}

int kmsBackend::getDRMFD() const
{
    return (drm_) ? drm_->fd : -1;
}

////////////////////////////////////////////////////////////////////////////////////
kmsOutput::kmsOutput(const kmsBackend& kms,  drmModeConnector* connector, drmModeEncoder* encoder, unsigned int id) : output(id), drmConnector_(connector), drmEncoder_(encoder)
{
    drmSavedCrtc_ = drmModeGetCrtc(kms.getDRMFD(), drmEncoder_->crtc_id);

    //todo: init modes correctly
    size_.x = drmConnector_->modes[0].hdisplay;
    size_.y = drmConnector_->modes[0].vdisplay;

    gbmSurface_ = gbm_surface_create(kms.getGBMDevice(), size_.x, size_.y, GBM_BO_FORMAT_XRGB8888, GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
    if(!gbmSurface_)
    {
        throw std::runtime_error("kmsOutput::kmsOutput: cant create gbmSurface");
        return;
    }
}

kmsOutput::~kmsOutput()
{
    releaseFB(fbs_[0]);
    releaseFB(fbs_[1]);

    gbm_surface_destroy(gbmSurface_);
}

void kmsOutput::resetCrtc()
{
    if(!drmSavedCrtc_ || !drmConnector_)
        return;

    drmModeSetCrtc(getKMSBackend()->getDRMFD(), drmSavedCrtc_->crtc_id, drmSavedCrtc_->buffer_id, drmSavedCrtc_->x, drmSavedCrtc_->y, &drmConnector_->connector_id, 1, &drmSavedCrtc_->mode);
}

void kmsOutput::render()
{
    if(flipping_)
        return;

    output::render();
    swapBuffers();
}

void kmsOutput::createFB(fb& obj)
{
    obj.buffer = gbm_surface_lock_front_buffer(gbmSurface_);

    unsigned int width = gbm_bo_get_width(obj.buffer);
    unsigned int height = gbm_bo_get_height(obj.buffer);
    unsigned int handle = gbm_bo_get_handle(obj.buffer).u32;
    unsigned int stride = gbm_bo_get_stride(obj.buffer);

    drmModeAddFB(getKMSBackend()->getDRMFD(), width, height, 24, 32, stride, handle, &obj.fb);

    if(!crtcActive_) drmModeSetCrtc(getKMSBackend()->getDRMFD(), drmEncoder_->crtc_id, obj.fb, 0, 0, &drmConnector_->connector_id, 1, &drmConnector_->modes[0]);
    crtcActive_ = 1;
}

void kmsOutput::releaseFB(fb& obj)
{
    if(obj.buffer) drmModeRmFB(getKMSBackend()->getDRMFD(), obj.fb);
    if(obj.fb) gbm_surface_release_buffer(gbmSurface_, obj.buffer);

    obj.buffer = nullptr;
    obj.fb = 0;
}


void kmsOutput::swapBuffers()
{
    if(!flipping_ && getKMSBackend() && iroSessionManager()->active())
    {
        releaseFB(fbs_[frontBuffer_]);
        createFB(fbs_[frontBuffer_]);

        drmModePageFlip(getKMSBackend()->getDRMFD(), drmEncoder_->crtc_id, fbs_[frontBuffer_].fb, DRM_MODE_PAGE_FLIP_EVENT, this);

        flipping_ = 1;
    }
}

void kmsOutput::wasFlipped()
{
    frontBuffer_ ^= 1;
    flipping_ = 0;

    releaseFB(fbs_[frontBuffer_]);
}

void kmsOutput::sendInformation(const outputRes& res) const
{
    //todo
    wl_output_send_scale(&res.getWlResource(), 1);
    wl_output_send_geometry(&res.getWlResource(), position_.x, position_.y, drmConnector_->mmWidth, drmConnector_->mmHeight, drmConnector_->subpixel, "unknown", "unknown", WL_OUTPUT_TRANSFORM_NORMAL);

    for(unsigned int i(0); i < (unsigned int) drmConnector_->count_modes; i++)
    {
        unsigned int flags = 0;
        wl_output_send_mode(&res.getWlResource(), flags, drmConnector_->modes[i].hdisplay, drmConnector_->modes[i].vdisplay, drmConnector_->modes[i].vrefresh * 1000);
    }
}
