#include <iro/compositor/callback.hpp>

callbackRes::callbackRes(wl_client& client, unsigned int id) : resource(client, id, nullptr, nullptr)
{
}
