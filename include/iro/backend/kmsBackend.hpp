#pragma once

#include <iro/include.hpp>
#include <iro/backend/backend.hpp>
#include <iro/backend/output.hpp>

#include <gbm.h>
#include <drm.h>
#include <xf86drmMode.h>
#include <libinput.h>

kmsBackend* getKMSBackend();

//backend
class kmsBackend : public backend
{
protected:
    friend int inputEventLoop(int fd, unsigned int mask, void* data);
    int inputEvent();

    device* drm_ = nullptr;
    gbm_device* gbmDevice_ = nullptr;
    libinput* input_ = nullptr;

    wl_event_source* drmEventSource_ = nullptr;
    wl_event_source* inputEventSource_ = nullptr;

    void onTTYEnter();
    void onTTYLeave();

    void onDRMPause();
    void onDRMResume();

public:
    kmsBackend();
    ~kmsBackend();

    virtual unsigned int getType() const override { return backendType::kms; }
    gbm_device* getGBMDevice() const { return gbmDevice_; }
    int getDRMFD() const;
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

    //cb
    friend void drmPageFlipEvent(int, unsigned int, unsigned int, unsigned int, void*);
    void wasFlipped();

protected:
    drmModeCrtc* drmSavedCrtc_ = nullptr;
    drmModeConnector* drmConnector_ = nullptr;
    drmModeEncoder* drmEncoder_ = nullptr;

    gbm_surface* gbmSurface_ = nullptr;

    bool flipping_ = 0;
    fb fbs_[2];
    unsigned char frontBuffer_ = 0;

    /////
    void releaseFB(fb& obj);
    void createFB(fb& obj);

    void resetCrtc();

public:
    kmsOutput(const kmsBackend& kms, drmModeConnector* connector, drmModeEncoder* encoder, unsigned int id);
    ~kmsOutput();

    virtual void* getNativeSurface() const override { return gbmSurface_; }
    virtual void sendInformation(const outputRes& res) const override;
};
