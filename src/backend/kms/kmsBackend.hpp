#pragma once

#include <iro.hpp>
#include <backend/backend.hpp>
#include <backend/output.hpp>
#include <backend/egl.hpp>

#include <gbm.h>
#include <drm.h>
#include <xf86drmMode.h>

kmsBackend* getKMSBackend();

class kmsBackend : public backend
{
protected:
    ttyHandler* tty_ = nullptr;
    inputHandler* input_ = nullptr;

    eglContext* eglContext_ = nullptr;

    int fd_;

    gbm_device* gbmDevice_ = nullptr;

    drmModeConnector* drmConnector_ = nullptr;
    drmModeEncoder* drmEncoder_ = nullptr;
    drmModeModeInfo drmMode_;

    drmModeCrtc* drmSavedCrtc_ = nullptr;

    wl_event_source* drmEventSource_ = nullptr;

    void onEnter();
    void onLeave();

public:
    kmsBackend();
    ~kmsBackend();

    backendType getType() const { return backendType::kms; }
    eglContext* getEglContext() const { return eglContext_; }
    ttyHandler* getTTYHandler() const { return tty_; }
    inputHandler* getInputHandler() const { return input_; }

    gbm_device* getGBMDevice() const { return gbmDevice_; }
    drmModeConnector* getDRMConnector() const { return drmConnector_; }
    drmModeEncoder* getDRMEncoder() const { return drmEncoder_; }
    const drmModeModeInfo& getDRMMode() const { return drmMode_; }

    int getFD() const { return fd_; }
};

class kmsOutput : public output
{
protected:
    struct fb
    {
        gbm_bo* buffer = nullptr;
        unsigned int fb = 0;
    };

protected:
    gbm_surface* gbmSurface_ = nullptr;
    bool flipping_ = 0;
    EGLSurface eglSurface_;

    fb fbs_[2];
    unsigned char frontBuffer_ = 0;

public:
    kmsOutput(const kmsBackend& kms, unsigned int id);
    ~kmsOutput();

    unsigned int getFB() const { return fbs_[frontBuffer_].fb; }

    void makeEglCurrent();
    void swapBuffers();

    void wasFlipped(){ flipping_ = 0; }

    vec2ui getSize() const;
};
