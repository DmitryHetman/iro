#include <iro/util/global.hpp>
#include <wayland-server-core.h>

namespace iro
{

Global::~Global()
{
	if(wlGlobal_) wl_global_destroy(wlGlobal_);
}

}
