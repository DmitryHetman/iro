#include "types.hpp"

surface::surface(wl_resource* res) : resource_(res)
{
}

void surface::commit()
{
    current_ = pending_;
    pending_ = surfaceState();
}
