#pragma once

#include <iro/include.hpp>
#include <ny/draw/gl/egl.hpp>
#include <nytl/nonCopyable.hpp>

#include <string>
#include <vector>
#include <memory>

namespace iro
{

///The eglContext class can be used to bind an egl conext to a
///wl_display, and to transform egl client buffers into openGL images.
///Since everything egl-realted is totally hidden inside and entirely handled by this class,
///it is intented to only have one WaylandEglContext at the moment. Can change in future. 
class WaylandEglContext : public ny::EglContext
{
protected:
	class Impl;
	std::unique_ptr<Impl> impl_;

public:
    ///Constructs the eglContext with a nativeDisplay Type. This can be e.g. a wl_display*
    ///(client-side!) or a Display (Xlib client side), as well as a kms display type (gbm_device).
    WaylandEglContext(void* nativeDisplay);
    virtual ~WaylandEglContext();

	///Binds the context to a wl_display. Returns 0 on failure. See the wayland wgl bind display
	///extension for more information about egl and wayland integration.
    bool bindWlDisplay(wl_display& disp);

	///Unbinds the context from the wl_display. Returns 0 on failue.
    bool unbindWlDisplay(wl_display& disp);

	///Queries the given attribute for the given buffer resource.
    int queryWlBuffer(wl_resource& buf, int attribute);

	///Creates an EGLImageKHR (void*) for the given buffer resource.
	///Target parameter defaulted to GL_TEXTURE_2D.
    void* createImageKHR(wl_resource& res, const int* attrib = nullptr, 
			unsigned int target = 0x31D5);

	///Creates an egl surface from a native window type.
	void* createSurface(void* window, const int* attrib = nullptr);
	void* createSurface(unsigned int window, const int* attrib = nullptr);

	///Makes the context current for a given egl surface. Shortcut for calling
	///EglContext::surface(surface) and then EglContext::makeCurrent().
	bool makeCurrentForSurface(void* eglSurface);

	///Destroy a given EglSurface which were created with createSurface or a call to 
	///eglCreateWindowSurface().
	void destroySurface(void* surface);

	///Destroys the given EGLImageKHR (void*).
    bool destroyImageKHR(void* image);

	///Is able to bind the given EGLImageKHR (void*) to a GL Texture.
    void imageTargetTexture(void* image, unsigned int target = 0x0DE1);
};

}
