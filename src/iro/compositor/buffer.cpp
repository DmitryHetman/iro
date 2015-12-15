#include <iro/compositor/buffer.hpp>
#include <wayland-server-protocol.h>

namespace iro
{

BufferRes::BufferRes(wl_resource& res) : Resource(res)
{
}

BufferRes::~BufferRes()
{
}

void BufferRes::sendRelease() const
{
	wl_buffer_send_release(&wlResource());
}

}
