#include <iro/backend/egl.hpp>

#include <nytl/make_unique.hpp>
#include <nytl/misc.hpp>
#include <ny/base/log.hpp>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <stdexcept>

namespace iro
{

//WaylandEglContext::Impl
//https://www.khronos.org/registry/gles/extensions/OES/OES_EGL_image_external.txt
class WaylandEglContext::Impl
{
public:
	PFNEGLBINDWAYLANDDISPLAYWL eglBindWaylandDisplayWL = nullptr;
	PFNEGLUNBINDWAYLANDDISPLAYWL eglUnbindWaylandDisplayWL = nullptr;
	PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL = nullptr;
	PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = nullptr;
	PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR = nullptr;
	void(*eglImageTargetTexture)(int, void*) = nullptr;
};

//WaylandEglContext
WaylandEglContext::WaylandEglContext(void* display) 
	: ny::EglContext(), impl_(nullptr)
{
	impl_ = nytl::make_unique<WaylandEglContext::Impl>();

	ny::EglContext::eglDisplay_ = eglGetDisplay((EGLNativeDisplayType)display);
    if(!eglDisplay())
    {
        throw std::runtime_error("could not get egl display");
        return;
    }

    EGLint major, minor;
    if(!eglInitialize(eglDisplay(), &major, &minor))
    {
        throw std::runtime_error("could not initalize egl");
        return;
    }

    if(!eglBindAPI(EGL_OPENGL_ES_API))
    {
        throw std::runtime_error("could not bind egl opengl es api");
        return;
    }

    const EGLint attr[] =
    {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 1,
        EGL_GREEN_SIZE, 1,
        EGL_BLUE_SIZE, 1,
        EGL_ALPHA_SIZE, 0,
        EGL_DEPTH_SIZE, 0,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

    EGLint configCount;
    if(!eglChooseConfig(eglDisplay(), attr, &eglConfig_, 1, &configCount) || 
			configCount != 1)
    {
        throw std::runtime_error("could not get egl config");
        return;
    }

	ny::EglContext::initEglContext(Api::openGLES);

    if(eglExtensionSupported("EGL_WL_bind_wayland_display") 
			&& eglExtensionSupported("EGL_KHR_image_base"))
    {
        impl_->eglCreateImageKHR = 
			(PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress("eglCreateImageKHR");
        impl_->eglDestroyImageKHR = 
			(PFNEGLDESTROYIMAGEKHRPROC) eglGetProcAddress("eglDestroyImageKHR");
        impl_->eglBindWaylandDisplayWL = 
			(PFNEGLBINDWAYLANDDISPLAYWL) eglGetProcAddress("eglBindWaylandDisplayWL");
        impl_->eglUnbindWaylandDisplayWL = 
			(PFNEGLUNBINDWAYLANDDISPLAYWL) eglGetProcAddress("eglUnbindWaylandDisplayWL");
        impl_->eglQueryWaylandBufferWL = 
			(PFNEGLQUERYWAYLANDBUFFERWL) eglGetProcAddress("eglQueryWaylandBufferWL");
        impl_->eglImageTargetTexture = (void(*)(int, void*)) 
			eglGetProcAddress("glEGLImageTargetTexture2DOES"); 

        if(!impl_->eglCreateImageKHR || !impl_->eglDestroyImageKHR || 
			!impl_->eglBindWaylandDisplayWL || !impl_->eglUnbindWaylandDisplayWL || 
			!impl_->eglQueryWaylandBufferWL)
        {
            throw std::runtime_error("could not load all needed egl extension functions");
            return;
        }
    }
    else
    {
        throw std::runtime_error("needed egl extensions extension not supported");
        return;
    }
}

WaylandEglContext::~WaylandEglContext()
{
	if(!eglDisplay_) return;
	if(eglContext_)
	{
		eglDestroyContext(eglDisplay_, eglContext_);
		eglContext_ = nullptr;
	}

	eglTerminate(eglDisplay());
	eglDisplay_ = nullptr;
}

bool WaylandEglContext::bindWlDisplay(wl_display& disp)
{
	if(!impl_->eglBindWaylandDisplayWL)
	{
		ny::sendWarning("WaylandEglContext::bindWlDisplay: function not loaded");
		return 0;
	}

	return impl_->eglBindWaylandDisplayWL(eglDisplay(), &disp);
}

bool WaylandEglContext::unbindWlDisplay(wl_display& disp)
{
	if(!impl_->eglUnbindWaylandDisplayWL)
	{
		ny::sendWarning("WaylandEglContext::unbindWlDisplay: function not loaded");
		return 0;
	}

	return impl_->eglUnbindWaylandDisplayWL(eglDisplay(), &disp);
}

int WaylandEglContext::queryWlBuffer(wl_resource& buf, int attribute)
{
	if(!impl_->eglQueryWaylandBufferWL)
	{
		ny::sendWarning("WaylandEglContext::queryWlBuffer: function not loaded");
		return 0;
	}

	int ret = 0;
	impl_->eglQueryWaylandBufferWL(eglDisplay(), &buf, attribute, &ret);

	return ret;
}

void* WaylandEglContext::createImageKHR(wl_resource& res, const int* attrib, unsigned int target)
{
	if(!impl_->eglCreateImageKHR)
	{
		ny::sendWarning("WaylandEglContext::eglCreateImageKHR: function not loaded");
		return nullptr;
	}

	return impl_->eglCreateImageKHR(eglDisplay(), eglContext(), target, &res, attrib);
}

bool WaylandEglContext::destroyImageKHR(void* image)
{
	if(!impl_->eglDestroyImageKHR)
	{
		ny::sendWarning("WaylandEglContext::eglDestroyImageKHR: function not loaded");
		return 0;
	}

	return impl_->eglDestroyImageKHR(eglDisplay(), image);
}

void WaylandEglContext::imageTargetTexture(void* image, unsigned int target)
{
	if(!impl_->eglImageTargetTexture)
	{
		ny::sendWarning("WaylandEglContext::eglImageTargetTexture: function not loaded");
		return;
	}

	impl_->eglImageTargetTexture(target, image);
}

bool WaylandEglContext::makeCurrentForSurface(void* eglsurf)
{
	eglSurface(eglsurf);
	return makeCurrent();
}

void* WaylandEglContext::createSurface(void* window, const int* attrib)
{
	return eglCreateWindowSurface(eglDisplay(), eglConfig(), 
			(EGLNativeWindowType) window, attrib);
}

void* WaylandEglContext::createSurface(unsigned int window, const int* attrib)
{
	return eglCreateWindowSurface(eglDisplay(), eglConfig(), 
			(EGLNativeWindowType) window, attrib);
}


void WaylandEglContext::destroySurface(void* surface)
{
	eglDestroySurface(eglDisplay(), surface);
}


}
