#include <iro/compositor/callback.hpp>

#include <wayland-server-protocol.h>

callbackRes::callbackRes(wl_client& client, unsigned int id) : resource(client, id, &wl_callback_interface, nullptr)
{
}
