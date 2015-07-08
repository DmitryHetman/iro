#include "wlInterface.hpp"
#include "types.hpp"
#include "x11Backend.hpp"
#include "server.hpp"
#include <iostream>

namespace wlInterface
{

//display
void displaySync(wl_client* client, wl_resource* resource, unsigned int callback)
{
    std::cout << "sync" << std::endl;
}
void displayGetRegistry(wl_client* client, wl_resource* resource, unsigned int registry)
{
    std::cout << "getReg" << std::endl;
}
void bindDisplay(wl_client *client, void *data, unsigned int version, unsigned int id)
{
     std::cout << "bindDisp" << std::endl;
}

//registry
void registryBind(struct wl_client* client, wl_resource* resource, unsigned int name, const char* interface, unsigned int version, unsigned int id)
{
    std::cout << "regBind" << std::endl;
}
void bindRegistry(wl_client *client, void *data, unsigned int version, unsigned int id)
{
    std::cout << "bindReg" << std::endl;
}

//compositor
void compositorCreateSurface(wl_client* client, wl_resource* resource, unsigned int id)
{
    struct wl_resource* res;
    if(!(res = wl_resource_create(client, &wl_surface_interface, 3, id)))
    {
        return;
    }

    //todo: delete the allocated object at surface destruction
    surface* data = new surface(res);
    wl_resource_set_implementation(res, &surfaceImplementation, data, NULL);
}
void compositorCreateRegion(wl_client* client, wl_resource* resource, unsigned int id)
{
}
void bindCompositor(wl_client *client, void *data, unsigned int version, unsigned int id)
{
    struct wl_resource* res;
    if(!(res = wl_resource_create(client, &wl_compositor_interface, version, id)))
    {
        return;
    }

    wl_resource_set_implementation(res, &compositorImplementation, data, NULL);
}


//subcompositor
void subcompositorDestroy(wl_client* client, wl_resource* resource)
{
}
void subcompositorGetSubsurface(wl_client* client, wl_resource* resource, unsigned int id, wl_resource* surface, wl_resource* parent)
{
}
void bindSubcompositor(wl_client* client, void* data, unsigned int version, unsigned int id)
{
    struct wl_resource* res;
    if(!(res = wl_resource_create(client, &wl_subcompositor_interface, version, id)))
    {
        return;
    }

    wl_resource_set_implementation(res, &subcompositorImplementation, data, NULL);
}


//shell
void shellGetShellSurface (wl_client* client, wl_resource* resource, unsigned int id, wl_resource* surface)
{
    struct wl_resource* res;
    if(!(res = wl_resource_create(client, &wl_shell_surface_interface, 1, id)))
    {
        return;
    }

    wl_resource_set_implementation(res, &shellSurfaceImplementation, getServer(), NULL);
}
void bindShell(wl_client* client, void* data, unsigned int version, unsigned int id)
{
    struct wl_resource* res;
    if(!(res = wl_resource_create(client, &wl_shell_interface, version, id)))
    {
        return;
    }

    wl_resource_set_implementation(res, &shellImplementation, data, NULL);
}

//surface
void surfaceDestroy(wl_client* client, wl_resource* resource)
{
}
void surfaceAttach(wl_client* client, wl_resource* resource, wl_resource* buffer, int x, int y)
{
    surface* s = static_cast<surface*>(resource->data);
    s->getPending().buffer = buffer;
    s->getPending().attached = 1;
    s->getPending().offset = vec2i(x, y);
}
void surfaceDamage(wl_client* client, wl_resource* resource, int x, int y, int width, int height)
{
    surface* s = static_cast<surface*>(resource->data);
    s->getPending().damage = rect2i(x, y, width, height);
}
void surfaceFrame(wl_client* client, wl_resource* resource, unsigned int callback)
{
}
void surfaceOpaqueRegion(wl_client* client,wl_resource* resource, wl_resource* region)
{
    rect2i* r = static_cast<rect2i*>(region->data);
    surface* s = static_cast<surface*>(resource->data);

    s->getPending().opaque = *r;
}
void surfaceInputRegion(wl_client* client,wl_resource* resource, wl_resource* region)
{
    rect2i* r = static_cast<rect2i*>(region->data);
    surface* s = static_cast<surface*>(resource->data);

    s->getPending().input = *r;
}
void surfaceCommit(wl_client* client, wl_resource* resource)
{
    surface* s = static_cast<surface*>(resource->data);
    s->commit();

    getServer()->getBackend()->renderSurface(s);
}
void surfaceBufferTransform(wl_client* client, wl_resource* resource, int transform)
{
}
void surfaceBufferScale(wl_client* client, wl_resource* resource, int scale)
{
}


//region
void regionDestroy(wl_client *client, wl_resource *resource)
{
}
void regionAdd(wl_client *client, wl_resource *resource, int x, int y, int width, int height)
{
}
void regionSubtract(wl_client *client, wl_resource *resource, int x, int y, int width, int height)
{
}

//shellSurface
void shellSurfacePong(wl_client* client, wl_resource* resource, unsigned int serial)
{
}
void shellSurfaceMove(wl_client* client, wl_resource* resource, wl_resource* seat, unsigned int serial)
{
}
void shellSurfaceResize(wl_client* client, wl_resource* resource, wl_resource* seat, unsigned int serial, unsigned int edges)
{
}
void shellSurfaceSetToplevel(wl_client* client, wl_resource* resource)
{
}
void shellSurfaceSetTransient(wl_client* client, wl_resource* resource, wl_resource* parent, int x, int y, unsigned int flags)
{
}
void shellSurfaceSetFullscreen(wl_client* client, wl_resource* resource, unsigned int method, unsigned int framerate, wl_resource* output)
{
}
void shellSurfaceSetPopup(wl_client* client, wl_resource* resource, wl_resource* seat, unsigned int serial, wl_resource* parent, int x, int y, unsigned int flags)
{
}
void shellSurfaceSetMaximized(wl_client* client, wl_resource* resource, wl_resource* output)
{
}
void shellSurfaceSetTitle(wl_client* client, wl_resource* resource, const char* title)
{
}
void shellSurfaceSetClass(wl_client* client, wl_resource* resource, const char* class_)
{
}


//dataOffer
void dataOfferAccept(wl_client* client, wl_resource* resource, unsigned int serial, const char* mime_type)
{
}
void dataOfferReceive(wl_client* client, wl_resource* resource, const char* mime_type, int fd)
{
}
void dataOfferDestroy(wl_client* client, wl_resource* resource)
{
}

//dataSource
void dataSourceOffer(wl_client* client, wl_resource* resource, const char* mime_type)
{
}
void dataSourceDestroy(wl_client* client, wl_resource* resource)
{
}

//dataDevice
void dataDeviceStartDrag(wl_client* client, wl_resource* resource, wl_resource* source, wl_resource* origin, wl_resource* icon, unsigned int serial)
{
}
void dataSeivceSetSelection(wl_client* client, wl_resource* resource, wl_resource* source, unsigned int serial)
{
}
void dataDeviceRelease(wl_client* client, wl_resource* resource)
{
}

//dataDeviceManager
void dataDeviceManagerCreateDataSource(wl_client* client, wl_resource* resource, unsigned int id)
{
}
void dataDeviceManagerGetDataDevice(wl_client* client, wl_resource* resource, unsigned int id, wl_resource* seat)
{
}


}
