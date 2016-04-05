#pragma once

#include <iro/include.hpp>

#include <ny/draw/gl/texture.hpp>

#include <nytl/nonCopyable.hpp>
#include <nytl/vec.hpp>

namespace iro
{

///Abstract base class that is implemented by the backend and is responsible for managing the 
///surface content.
class SurfaceContext : public nytl::NonCopyable
{
public:
	virtual ~SurfaceContext() = default;
	virtual bool attachBuffer(BufferRes& buf, nytl::Vec2ui& size) = 0;
	virtual const ny::Texture* content() const = 0;
};

///Default implementation for a surfaceContext, capable of using egl and shm buffers.
class DefaultSurfaceContext : public SurfaceContext
{
protected:
	WaylandEglContext* eglContext_ = nullptr;
	ny::GlTexture texture_;

	bool attachShmBuffer(wl_shm_buffer& shmBuffer, nytl::Vec2ui& size);
	bool attachEglBuffer(wl_resource& eglBuffer, nytl::Vec2ui& size);

public:
	DefaultSurfaceContext(WaylandEglContext& ctx);
	virtual ~DefaultSurfaceContext();

	virtual bool attachBuffer(BufferRes& buf, nytl::Vec2ui& size) override;
	virtual const ny::Texture* content() const override;
};

}
