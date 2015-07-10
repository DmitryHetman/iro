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
    gbm_surface* gbmSurface_ = nullptr;
    gbm_bo* gbmBuffer_ = nullptr;

    bool flipping_ = 0;

    unsigned int fbID_ = 0;

    EGLSurface eglSurface_;

public:
    kmsOutput(const kmsBackend& kms, unsigned int id);
    ~kmsOutput();

    unsigned int getFBID() const { return fbID_; }

    void makeEglCurrent();
    void swapBuffers();

    void wasFlipped(){ flipping_ = 0; }

    vec2ui getSize() const;
};
