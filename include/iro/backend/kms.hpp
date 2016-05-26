#pragma once

#include <iro/include.hpp>
#include <iro/backend/backend.hpp>
#include <iro/backend/output.hpp>

#include <gbm.h>
#include <xf86drmMode.h>

#include <memory>

namespace iro
{

///Implements the backend interface for the kms/drm/gdm backend.
///Can only be used when started from command line. Usually requires
///session and tty management to work properly. Does not include input, use
///inputManager for a libinput seat backend.
class KmsBackend : public Backend
{
protected:
    static int inputCallback(int fd, unsigned int mask, void* data);

protected:
    int inputEvent();

	Compositor* compositor_;
    Device* drm_ = nullptr;
    gbm_device* gbmDevice_ = nullptr;
    wl_event_source* drmEventSource_ = nullptr;
	std::unique_ptr<WaylandEglContext> eglContext_;

    void onTerminalEnter();
    void onTerminalLeave();

    //void onDRMPause();
    //void onDRMResume();

public:
    KmsBackend(Compositor& comp, DeviceHandler& dev);
    virtual ~KmsBackend();

    gbm_device* gbmDevice() const { return gbmDevice_; }
    Device& drmDevice() const { return *drm_; }

	Compositor& compositor() const { return *compositor_; }
	void setCallbacks(TerminalHandler& tty);

	virtual std::unique_ptr<SurfaceContext> createSurfaceContext() const override;
	virtual WaylandEglContext* eglContext() const override { return eglContext_.get(); }
};

struct DrmOutput;

///Implements the output interface for the kms backend.
class KmsOutput : public Output
{
public:
	KmsOutput(KmsBackend& b, const DrmOutput& info);
	virtual ~KmsOutput();

	KmsBackend& backend() const { return *backend_; }

    void resetCrtc();
	void setCrtc();

    virtual void sendInformation(const OutputRes& res) const override;
	virtual void redraw() override;

protected:
    struct Framebuffer
    {
        gbm_bo* buffer = nullptr;
        unsigned int fb = 0;

        bool valid(){ return (buffer != nullptr && fb != 0); }
    };

	friend int drmEvent(int, unsigned int, void*);
	static void drmPageFlipEvent(int, unsigned int, unsigned int, unsigned int, void*);

protected:
	KmsBackend* backend_ = nullptr;
    drmModeCrtc* drmSavedCrtc_ = nullptr;
    drmModeConnector* drmConnector_ = nullptr;
    drmModeEncoder* drmEncoder_ = nullptr;
	drmModeCrtc* drmCrtc_ = nullptr;
    gbm_surface* gbmSurface_ = nullptr;
	void* eglSurface_ = nullptr;

    bool flipping_ = 0;
    Framebuffer fbs_[2];
	Framebuffer* current_ = nullptr;
	Framebuffer* next_ = nullptr;
    bool crtcActive_ = 0;

	//functions
    void releaseFB(Framebuffer& obj);
    void createFB(Framebuffer& obj);

    void swapBuffers();
	void flipped();
};

}
