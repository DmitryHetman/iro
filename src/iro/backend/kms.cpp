#include <iro/backend/kms.hpp>
#include <iro/backend/egl.hpp>
#include <iro/backend/surfaceContext.hpp>
#include <iro/backend/devices.hpp>
#include <iro/backend/tty.hpp>
#include <iro/compositor/compositor.hpp>

#include <ny/base/log.hpp>
#include <nytl/misc.hpp>

#include <ny/draw/gl/drawContext.hpp>
#include <ny/base/log.hpp>

#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

#include <gbm.h>
#include <drm.h>
#include <xf86drm.h> 
#include <fcntl.h>

#include <cstring>

	namespace iro
	{

	struct DrmOutputMode
	{
		unsigned int width;
		unsigned int height;
		unsigned int refresh;
		unsigned int type;
		unsigned int flags;
		std::string name;
	};

	struct DrmOutput
	{
		drmModeConnector* connector;
		drmModeEncoder* encoder;
		drmModeCrtc* crtc;
		unsigned int id;
		std::vector<DrmOutputMode> modes;
	};

	std::vector<DrmOutput> queryInformation(int fd)
	{
		std::vector<DrmOutput> ret;
		drmModeRes* resources = drmModeGetResources(fd);

		auto conCount = static_cast<unsigned int>(resources->count_connectors);
		auto encCount = static_cast<unsigned int>(resources->count_encoders);
		auto crtCount = static_cast<unsigned int>(resources->count_crtcs);
	 
		ny::sendLog(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
		ny::sendLog("KmsBackend::KmsBackend: Begininning to query outputs");
		ny::sendLog("drmResources contain ", conCount, " connectors");
		ny::sendLog("drmResources contain ", encCount, " encoders");
		ny::sendLog("drmResources contain ", crtCount, " crtcs");
		ny::sendLog("drmResources contain ", resources->count_fbs, " fbs");

		for(auto c = 0u; c < conCount; c++)
		{
			auto connector = drmModeGetConnector(fd, resources->connectors[c]);
			if(!connector)
			{
				ny::log("Failed to get connector ", c, ", ", resources->connectors[c]);
				continue;
			}

			if(connector->connection != DRM_MODE_CONNECTED)
			{
				ny::log("Connector ", c, " is not connected.");
				continue;
			}

			auto conEncCount = static_cast<unsigned int>(connector->count_encoders);
			ny::log("Found ", conEncCount, " encoder for connector ", connector);
			for(auto e = 0u; e < conEncCount; ++e)
			{
				auto encoder = drmModeGetEncoder(fd, connector->encoders[e]);
				if(!encoder)
				{
					ny::log("Failed to get encoder for ", e, ", ", connector->encoders[e]);
					continue;
				}

				ny::log("Found matching connector/encoder pair: ", connector, " & ", encoder);
				auto crtc = drmModeGetCrtc(fd, encoder->crtc_id);

				if(!crtc)
				{
					ny::log("Failed to get crtc for id ", encoder->crtc_id);
					continue;
				}

				ny::log("Succefully retrieved ctrc ", crtc, " with id ", encoder->crtc_id);
				auto modeCount = static_cast<unsigned int>(connector->count_modes);
				if(!modeCount)
				{
					ny::log("Found no modes for pair");
					continue;
				}

				DrmOutput output;
				output.connector = connector;
				output.encoder = encoder;
				output.crtc = crtc;
				output.id = ret.size();
				
				output.modes.resize(modeCount);

				ny::log("Found ", modeCount, " matching modes for pair");
				for(auto m = 0u; m < modeCount; ++m)
				{
					const auto& mode = connector->modes[m];
					output.modes[m].height = mode.vdisplay;
					output.modes[m].width = mode.hdisplay;
					output.modes[m].refresh = mode.vrefresh;
					output.modes[m].flags = mode.flags;
					output.modes[m].type = mode.type;
					output.modes[m].name = mode.name;

					ny::log("Mode ", m, ": ", mode.name, 
						"\n\twidth: ", mode.hdisplay, "\n\theight: ", mode.vdisplay,
						"\n\trefresh: ", mode.vrefresh, "\n\ttype: ", mode.type,
						"\n\tflags: ", mode.flags);
				}

				ret.push_back(output);
			}
		}

	ny::log("End of drm querying. Could find ", ret.size(), " valid drm outputs!");
	ny::log(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
	return ret;
}


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

	auto infos = queryInformation(drm_->fd());
    if(infos.empty())
    {
        throw std::runtime_error("kmsBackend::kmsBackend: unable to create an output");
        return;
    }

	for(auto& info : infos)
	{
		ny::log("mc: ", info.modes.size());
		auto output = std::make_unique<KmsOutput>(*this, info);
		addOutput(std::move(output));
	}

	//drm events
    drmEventSource_ = 
		wl_event_loop_add_fd(&comp.wlEventLoop(), drm_->fd(), WL_EVENT_READABLE, drmEvent, this);
}

KmsBackend::~KmsBackend()
{
	for(auto i = 0u; i < outputs_.size();)
		if(!destroyOutput(*outputs_[i])) ++i;

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
KmsOutput::KmsOutput(KmsBackend& kms, const DrmOutput& output) 
	: Output(kms.compositor(), output.id), backend_(&kms)
{
	drmEncoder_ = output.encoder;
	drmConnector_ = output.connector;
	drmCrtc_ = output.crtc;
    drmSavedCrtc_ = drmModeGetCrtc(kms.drmDevice().fd(), drmEncoder_->crtc_id);

	if(id() == 1) position_ = {1920, 0};

    //todo: init modes correctly
    size_.x = output.modes[0].width;
    size_.y = output.modes[0].height;

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
	current_ = &fbs_[0];
	next_ = &fbs_[1];
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

	auto* s = drmSavedCrtc_;
    drmModeSetCrtc(backend().drmDevice().fd(), s->crtc_id, s->buffer_id, s->x, s->y, 
		&drmConnector_->connector_id, 1, &s->mode);
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

	ny::sendLog("KmsOutput::eglSurface = ", eglSurface_);
	ny::sendLog("EglContext::eglSurface = ", backend().eglContext()->eglSurface());

	if(!backend().eglContext()->apply())
	{
		ny::sendWarning("KmsOutput::redraw: failed to swap egl buffers");
		scheduleRepaint();
		return;
	}

	swapBuffers();
	
	backend().eglContext()->makeNotCurrent();
}

void KmsOutput::createFB(Framebuffer& obj)
{
    obj.buffer = gbm_surface_lock_front_buffer(gbmSurface_);

	if(!obj.buffer) throw std::runtime_error("gbm_surface_lock_front_buffer failed");

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

void KmsOutput::releaseFB(Framebuffer& obj)
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
        releaseFB(*current_);
        createFB(*current_);

        drmModePageFlip(backend().drmDevice().fd(), drmEncoder_->crtc_id, current_->fb, 
				DRM_MODE_PAGE_FLIP_EVENT, this);
        flipping_ = 1;
    }
}

void KmsOutput::flipped()
{
	ny::sendLog("KmsOutput::flipped");
    flipping_ = 0;
	std::swap(current_, next_);

    releaseFB(*current_);
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
