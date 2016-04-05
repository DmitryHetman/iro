#include <iro/backend/kms.hpp>
#include <iro/backend/egl.hpp>
#include <iro/backend/surfaceContext.hpp>
#include <iro/backend/devices.hpp>
#include <iro/backend/tty.hpp>
#include <iro/compositor/compositor.hpp>

#include <ny/base/log.hpp>
#include <nytl/misc.hpp>

#include <ny/draw/gl/drawContext.hpp>

#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

#include <gbm.h>
#include <drm.h>
#include <xf86drm.h> 
#include <fcntl.h>

#include <cstring>

namespace iro
{

//drmCallbacks
void KmsOutput::drmPageFlipEvent(int, unsigned int, unsigned int, unsigned int, void *data)
{
	if(!data) return;
    static_cast<KmsOutput*>(data)->flipped();
}

int drmEvent(int fd, unsigned int, void*)
{
    drmEventContext ev;
	std::memset(&ev, 0, sizeof(ev));
    ev.version = DRM_EVENT_CONTEXT_VERSION;
    ev.page_flip_handler = KmsOutput::drmPageFlipEvent;
    drmHandleEvent(fd, &ev);

    return 0;
}

//KmsBackend
KmsBackend::KmsBackend(Compositor& comp, DeviceHandler& dev)
	: Backend(), compositor_(&comp)
{
    std::string path = "/dev/dri/card0";
    drm_ = dev.takeDevice(path, O_RDWR);

    if(!drm_ || !drm_->fd())
    {
        throw std::runtime_error("KmsBackend::KmsBackend: failed to get drm device");
        return;
    }

	drmSetMaster(drm_->fd());
	drm_->onPause([=]{ ny::sendLog("drm pause"); drmDropMaster(drm_->fd()); });
	drm_->onResume([=]{ ny::sendLog("drm resume"); drmSetMaster(drm_->fd()); });

    gbmDevice_ = gbm_create_device(drm_->fd());
    if(!gbmDevice_)
    {
        throw std::runtime_error("kmsBackend::kmsBackend: cant create gbm device");
        return;
    }

	//egl
	eglContext_ = std::make_unique<WaylandEglContext>(gbmDevice_);
	if(!eglContext_)
	{
        throw std::runtime_error("KmsBackend::KmsBackend: failed to create EglContext");
        return;
	}
	eglContext_->bindWlDisplay(compositor_->wlDisplay());

	//drm resources
    drmModeRes* resources = drmModeGetResources(drm_->fd());
    if (!resources)
    {
        throw std::runtime_error("KmsBackend::KmsBackend: drmModeGetResources failed");
        return;
    }

	//find and create outputs
    unsigned int c, e; //connector, encoder
    drmModeConnector* connector = nullptr;
    drmModeEncoder* encoder = nullptr;

	ny::sendLog(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
	ny::sendLog("KmsBackend::KmsBackend: Begininning to query outputs");
	ny::sendLog("drmResources contain ", resources->count_connectors, " connectors");
	ny::sendLog("drmResources contain ", resources->count_encoders, " encoders");

    for (c = 0; c < static_cast<unsigned int>(resources->count_connectors); c++)
    {
        if(!(connector = drmModeGetConnector(drm_->fd(), resources->connectors[c])))
		{
			ny::sendLog("failed to get connector ", resources->connectors[c]);
			continue;
		}

        if (connector->connection == DRM_MODE_CONNECTED && connector->count_modes > 0)
        {
            for (e = 0; e < static_cast<unsigned int>(resources->count_encoders); e++)
            {
                if(!(encoder = drmModeGetEncoder(drm_->fd(), resources->encoders[e])))
				{
					ny::sendLog("failed to get encoder ", resources->encoders[e]);
					continue;
				}

                if (encoder->encoder_id == connector->encoder_id)
                {
					//create output
					ny::sendLog(">>matching connector/encoder pair ", outputs_.size());
                    addOutput(std::make_unique<KmsOutput>(*this, connector, encoder, 
								outputs_.size()));
                }
				else
				{
					ny::sendLog(encoder, "and ", connector ,": different encoders");
					drmModeFreeEncoder(encoder);
				}
            }
        }
		else
		{
			ny::sendLog("connector ", connector, " invalid");
			drmModeFreeConnector(connector);
		}
    }

	ny::sendLog("End of querying kms outputs: created ", outputs_.size());
	ny::sendLog(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");

    if (outputs_.size() == 0)
    {
        throw std::runtime_error("kmsBackend::kmsBackend: unable to create an output");
        return;
    }

	//drm events
    drmEventSource_ = 
		wl_event_loop_add_fd(&comp.wlEventLoop(), drm_->fd(), WL_EVENT_READABLE, drmEvent, this);
}

KmsBackend::~KmsBackend()
{
	for(auto& outp : outputs_)
		destroyOutput(*outp);

	if(eglContext_)
	{
		eglContext_->unbindWlDisplay(compositor_->wlDisplay());
		eglContext_.reset();
	}

    if(drmEventSource_) wl_event_source_remove(drmEventSource_);
    if(gbmDevice_) gbm_device_destroy(gbmDevice_);
    if(drm_)
	{
		drmDropMaster(drm_->fd());
		drm_->release();
		drm_ = nullptr;
	}
}

std::unique_ptr<SurfaceContext> KmsBackend::createSurfaceContext() const
{
	return std::make_unique<DefaultSurfaceContext>(*eglContext_);
}

void KmsBackend::setCallbacks(TerminalHandler& handler)
{
	handler.beforeEnter(nytl::memberCallback(&KmsBackend::onTerminalEnter, this));
	handler.beforeLeave(nytl::memberCallback(&KmsBackend::onTerminalLeave, this));
}

void KmsBackend::onTerminalEnter()
{
	for(auto& outp: outputs_)
	{
		auto* o = static_cast<KmsOutput*>(outp.get());
		o->setCrtc();
		//o->scheduleRepaint();
		o->redraw();
	}
}

void KmsBackend::onTerminalLeave()
{
	for(auto& outp: outputs_)
	{
		auto* o = static_cast<KmsOutput*>(outp.get());
		o->resetCrtc();
	}
}

//output
KmsOutput::KmsOutput(KmsBackend& kms, drmModeConnector* c, drmModeEncoder* e, unsigned int id) 
	: Output(kms.compositor(), id), backend_(&kms), drmConnector_(c), drmEncoder_(e)
{
    drmSavedCrtc_ = drmModeGetCrtc(kms.drmDevice().fd(), drmEncoder_->crtc_id);

    //todo: init modes correctly
    size_.x = drmConnector_->modes[0].hdisplay;
    size_.y = drmConnector_->modes[0].vdisplay;

	//gbm
    gbmSurface_ = gbm_surface_create(kms.gbmDevice(), size_.x, size_.y, 
			GBM_BO_FORMAT_XRGB8888, GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
    if(!gbmSurface_)
    {
        throw std::runtime_error("kmsOutput::kmsOutput: cant create gbmSurface");
        return;
    }

	//egl
	if(!backend().eglContext())
	{
		throw std::runtime_error("KmsOutput::KmsOutput: backend ha no valid eglcontext");
		return;
	}

	eglSurface_ = backend().eglContext()->createSurface(gbmSurface_);
	if(!eglSurface_)
	{
		throw std::runtime_error("KmsOutput::KmsOutput: failed to create egl surface");
		return;
	}

	drawContext_ = std::make_unique<ny::GlDrawContext>();
	if(!drawContext_)
	{
		throw std::runtime_error("KmsOutput::KmsOutput: failed to create ny::GlDC");
		return;
	}

	scheduleRepaint();
}

KmsOutput::~KmsOutput()
{
	resetCrtc();
    if(fbs_[0].valid())releaseFB(fbs_[0]);
    if(fbs_[1].valid())releaseFB(fbs_[1]);
    if(gbmSurface_)gbm_surface_destroy(gbmSurface_);
}

void KmsOutput::setCrtc()
{
	crtcActive_ = 0;
}

void KmsOutput::resetCrtc()
{
    if(!drmSavedCrtc_ || !drmConnector_)
	{
		ny::sendWarning("KmsOutput::resetCrtc: invalid crtc or connector");
		return;
	}

    drmModeSetCrtc(backend().drmDevice().fd(), drmSavedCrtc_->crtc_id, drmSavedCrtc_->buffer_id, 
		drmSavedCrtc_->x, drmSavedCrtc_->y, &drmConnector_->connector_id, 1, &drmSavedCrtc_->mode);
}

void KmsOutput::redraw()
{
	ny::sendLog("KmsOutput:redraw: id ", id());
    if(flipping_)
	{
		ny::sendWarning("KmsOutput::redraw: buffers are flipping.");
		return;
	}

	if(!backend().eglContext()->makeCurrentForSurface(eglSurface_))
	{
		ny::sendWarning("KmsOutput::redraw: failed to make eglContext current");
		return;
	}

	Output::redraw();
	if(!backend().eglContext()->apply())
	{
		ny::sendWarning("KmsOutput::redraw: failed to swap egl buffers");
	}

	swapBuffers();
	
	backend().eglContext()->makeNotCurrent();
}

void KmsOutput::createFB(fb& obj)
{
    obj.buffer = gbm_surface_lock_front_buffer(gbmSurface_);

    unsigned int width = gbm_bo_get_width(obj.buffer);
    unsigned int height = gbm_bo_get_height(obj.buffer);
    unsigned int handle = gbm_bo_get_handle(obj.buffer).u32;
    unsigned int stride = gbm_bo_get_stride(obj.buffer);

    drmModeAddFB(backend().drmDevice().fd(), width, height, 24, 32, stride, handle, &obj.fb);

    if(!crtcActive_) 
	{
		drmModeSetCrtc(backend().drmDevice().fd(), drmEncoder_->crtc_id, obj.fb, 0, 0, 
				&drmConnector_->connector_id, 1, &drmConnector_->modes[0]);
		crtcActive_ = 1;
	}
}

void KmsOutput::releaseFB(fb& obj)
{
    if(obj.buffer) drmModeRmFB(backend().drmDevice().fd(), obj.fb);
    if(obj.fb) gbm_surface_release_buffer(gbmSurface_, obj.buffer);

    obj.buffer = nullptr;
    obj.fb = 0;
}


void KmsOutput::swapBuffers()
{
    if(!flipping_)
    {
        releaseFB(fbs_[frontBuffer_]);
        createFB(fbs_[frontBuffer_]);

        drmModePageFlip(backend().drmDevice().fd(), drmEncoder_->crtc_id, fbs_[frontBuffer_].fb, 
				DRM_MODE_PAGE_FLIP_EVENT, this);
        flipping_ = 1;
    }
}

void KmsOutput::flipped()
{
	ny::sendLog("KmsOutput::flipped");
    frontBuffer_ ^= 1;
    flipping_ = 0;

    releaseFB(fbs_[frontBuffer_]);
}

void KmsOutput::sendInformation(const OutputRes& res) const
{
    //todo
    wl_output_send_scale(&res.wlResource(), 1);
    wl_output_send_geometry(&res.wlResource(), position_.x, position_.y, drmConnector_->mmWidth, 
			drmConnector_->mmHeight, drmConnector_->subpixel, "unknown", "unknown", 
			WL_OUTPUT_TRANSFORM_NORMAL);

    for(unsigned int i(0); i < (unsigned int) drmConnector_->count_modes; i++)
    {
        unsigned int flags = 0;
        wl_output_send_mode(&res.wlResource(), flags, drmConnector_->modes[i].hdisplay, 
				drmConnector_->modes[i].vdisplay, drmConnector_->modes[i].vrefresh * 1000);
    }

	wl_output_send_done(&res.wlResource());
}

}
