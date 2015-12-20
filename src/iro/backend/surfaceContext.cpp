#include <iro/backend/surfaceContext.hpp>
#include <iro/backend/egl.hpp>
#include <iro/compositor/buffer.hpp>

#include <nytl/log.hpp>
#include <ny/draw/include.hpp>

//#ifdef TODO
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <glpbinding/glp20/glp.h>
using namespace glp20;

#include <wayland-server-core.h>

namespace iro
{

DefaultSurfaceContext::DefaultSurfaceContext(WaylandEglContext& ctx) : eglContext_(&ctx)
{
}

DefaultSurfaceContext::~DefaultSurfaceContext()
{
}

bool DefaultSurfaceContext::attachShmBuffer(wl_shm_buffer& shmBuffer, nytl::vec2ui& size)
{
	//todo: correct format and stuff...
    //unsigned int format = wl_shm_buffer_get_format(&shmBuffer);
    //unsigned int pitch = wl_shm_buffer_get_stride(&shmBuffer) / 4;

	size.x = wl_shm_buffer_get_width(&shmBuffer);
	size.y = wl_shm_buffer_get_height(&shmBuffer);

	nytl::sendLog("shm surface with size ", size);

    //data
    wl_shm_buffer_begin_access(&shmBuffer);
	unsigned char* data = static_cast<unsigned char*>(wl_shm_buffer_get_data(&shmBuffer));
	if(!data)
	{
		wl_shm_buffer_end_access(&shmBuffer);
		nytl::sendWarning("defaultSurfaceContext::attachShmBuffer: no data. "
				"buffer: ", &shmBuffer);
		return 0;
	}

    wl_shm_buffer_end_access(&shmBuffer);

	//texture
	eglContext_->makeCurrent();
	texture_.create(size, data, ny::Image::Format::rgba8888);

	eglContext_->makeNotCurrent();

	return 1;
}

bool DefaultSurfaceContext::attachEglBuffer(wl_resource& eglBuffer, nytl::vec2ui& size)
{
	eglContext_->makeCurrent();
    size.x = eglContext_->queryWlBuffer(eglBuffer, EGL_WIDTH);
    size.y = eglContext_->queryWlBuffer(eglBuffer, EGL_HEIGHT);
	unsigned int format = eglContext_->queryWlBuffer(eglBuffer, EGL_TEXTURE_FORMAT);

	nytl::sendLog("egl surface with size ", size, " format ", format);

	if(!texture_.glTexture())
	{
		GLuint tex;
		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &tex);

		texture_.glTexture(tex, size);
		texture_.bind();

		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int) GL_CLAMP_TO_EDGE);
		/////glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int) GL_CLAMP_TO_EDGE);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(GL_REPEAT));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(GL_REPEAT));

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(GL_LINEAR));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(GL_LINEAR));

	}
	else
	{
		texture_.bind();
	}

	nytl::sendLog("egl texture: ", texture_.glTexture());
        
	EGLint attribs[] = {EGL_WAYLAND_PLANE_WL, 0, EGL_NONE};
    void* img = eglContext_->createImageKHR(eglBuffer, attribs, EGL_WAYLAND_BUFFER_WL);

	if(!img)
	{
		nytl::sendWarning("DefaultSurfaceContext::attachEglBuffer: failed to create egl image");
		return 0;
	}

	eglContext_->imageTargetTexture(img, static_cast<unsigned int>(GL_TEXTURE_2D));
	//eglContext_->makeNotCurrent();

	return 1;
}

bool DefaultSurfaceContext::attachBuffer(BufferRes& buf, nytl::vec2ui& size)
{
    wl_resource& res = buf.wlResource();

    wl_shm_buffer* shm = wl_shm_buffer_get(&res);
    if(shm) //is shmBuffer
    {
		return attachShmBuffer(*shm, size);
    }
    else
	{
		int eglFormat = eglContext_->queryWlBuffer(buf.wlResource(), EGL_TEXTURE_FORMAT);
		if(!eglFormat)
		{
			//its not egl -> unkown format
			nytl::sendWarning("DefaultSurfaceContext::attachBuffer: invalid buffer format.");
			return 0;
		}

		return attachEglBuffer(buf.wlResource(), size);
    }
}

const ny::Texture* DefaultSurfaceContext::content() const
{
	return &texture_;
}

}
