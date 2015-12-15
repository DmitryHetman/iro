#include <iro/compositor/callback.hpp>

#include <wayland-server-protocol.h>

namespace iro
{

CallbackRes::CallbackRes(wl_client& client, unsigned int id) 
	: Resource(client, id, &wl_callback_interface, nullptr)
{
}

}
