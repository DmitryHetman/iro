#include <resources/region.hpp>

void regionDestroy(wl_client* client, wl_resource* resource)
{

}
void regionAdd(wl_client* client, wl_resource* resource, int x, int y, int width, int height)
{
    region& r = ((regionRes*) wl_resource_get_user_data(resource))->getRegion();
    r.add(x, y, width, height);
}
void regionSubtract(wl_client* client, wl_resource* resource, int x, int y, int width, int height)
{
    region& r = ((regionRes*) wl_resource_get_user_data(resource))->getRegion();
    r.subtract(x, y, width, height);
}

const struct wl_region_interface regionImplementation
{
    &regionDestroy,
    &regionAdd,
    &regionSubtract
};

////////////////////////////////////////7

regionRes::regionRes(wl_client* client, unsigned int id)
{
    wlResource_ = wl_resource_create(client, &wl_region_interface, 3, id);
    if(!wlResource_)
    {
        throw std::runtime_error("failed to create wayland surface");
        return;
    }

    wl_resource_set_implementation(wlResource_, &regionImplementation, this, nullptr);
}

