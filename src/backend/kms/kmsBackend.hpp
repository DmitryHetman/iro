#pragma once

#include <iro.hpp>
#include <backend/backend.hpp>
#include <backend/output.hpp>
#include <backend/egl.hpp>
#include <backend/session.hpp>

#include <gbm.h>
#include <drm.h>
#include <xf86drmMode.h>

kmsBackend* getKMSBackend();

//backend
class kmsBackend : public backend
{
protected:
    sessionHandler* session_ = nullptr;
    ttyHandler* tty_ = nullptr;
    inputHandler* input_ = nullptr;

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
    ttyHandler* getTTYHandler() const { return tty_; }
    inputHandler* getInputHandler() const { return input_; }

    gbm_device* getGBMDevice() const { return gbmDevice_; }
    drmModeConnector* getDRMConnector() const { return drmConnector_; }
    drmModeEncoder* getDRMEncoder() const { return drmEncoder_; }
    const drmModeModeInfo& getDRMMode() const { return drmMode_; }
    drmModeModeInfo& getDRMMode() { return drmMode_; }

    int getFD() const { return fd_; }
};

//output
class kmsOutput : public output
{
protected:
    struct fb
    {
        gbm_bo* buffer = nullptr;
        unsigned int fb = 0;

        bool valid(){ return (buffer != nullptr && fb != 0); }
    };

protected:
    gbm_surface* gbmSurface_ = nullptr;
    EGLSurface eglSurface_;

    bool set = 0;
    bool flipping_ = 0;

    fb fbs_[2];
    unsigned char frontBuffer_ = 0;

    /////
    void releaseFB(fb& obj);
    void createFB(fb& obj);

    void render();

public:
    kmsOutput(const kmsBackend& kms, unsigned int id);
    ~kmsOutput();

    void wasFlipped();
    void setCrtc();

    //output
    virtual void swapBuffers() override;
    virtual vec2ui getSize() const override;
    virtual EGLSurface getEglSurface() const override { return eglSurface_; }
};
