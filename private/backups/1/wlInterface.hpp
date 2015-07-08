#pragma once

#include <wayland-server-protocol.h>

class server;
server* getServer();

namespace wlInterface
{

//display
void displaySync(wl_client* client, wl_resource* resource, unsigned int callback);
void displayGetRegistry(wl_client* client, wl_resource* resource, unsigned int registry);
void bindDisplay(wl_client* client, void* data, unsigned int version, unsigned int id);

//registry
void registryBind(wl_client* client, wl_resource* resource, unsigned int name, const char* interface, unsigned int version, unsigned int id);
void bindRegistry(wl_client* client, void* data, unsigned int version, unsigned int id);

//compositor
void compositorCreateSurface(wl_client* client, wl_resource* resource, unsigned int id);
void compositorCreateRegion(wl_client* client, wl_resource* resource, unsigned int id);
void bindCompositor(wl_client* client, void* data, unsigned int version, unsigned int id);

//subcompositor
void subcompositorDestroy(wl_client* client, wl_resource* resource);
void subcompositorGetSubsurface(wl_client* client, wl_resource* resource, unsigned int id, wl_resource* surface, wl_resource* parent);
void bindSubcompositor(wl_client* client, void* data, unsigned int version, unsigned int id);

//shell
void shellGetShellSurface (wl_client* client, wl_resource* resource, unsigned int id, wl_resource* surface);
void bindShell(wl_client* client, void* data, unsigned int version, unsigned int id);

//surface
void surfaceDestroy(wl_client* client, wl_resource* resource);
void surfaceAttach(wl_client* client, wl_resource* resource, wl_resource* buffer, int x, int y);
void surfaceDamage(wl_client* client, wl_resource* resource, int x, int y, int width, int height);
void surfaceFrame(wl_client* client, wl_resource* resource, unsigned int callback);
void surfaceOpaqueRegion(wl_client* client,wl_resource* resource, wl_resource* region);
void surfaceInputRegion(wl_client* client,wl_resource* resource, wl_resource* region);
void surfaceCommit(wl_client* client, wl_resource* resource);
void surfaceBufferTransform(wl_client* client, wl_resource* resource, int transform);
void surfaceBufferScale(wl_client* client, wl_resource* resource, int scale);


//region
void regionDestroy(wl_client* client, wl_resource* resource);
void regionAdd(wl_client* client, wl_resource* resource, int x, int y, int width, int height);
void regionSubtract(wl_client* client, wl_resource* resource, int x, int y, int width, int height);

//shellsurface
void shellSurfacePong(wl_client* client, wl_resource* resource, unsigned int serial);
void shellSurfaceMove(wl_client* client, wl_resource* resource, wl_resource* seat, unsigned int serial);
void shellSurfaceResize(wl_client* client, wl_resource* resource, wl_resource* seat, unsigned int serial, unsigned int edges);
void shellSurfaceSetToplevel(wl_client* client, wl_resource* resource);
void shellSurfaceSetTransient(wl_client* client, wl_resource* resource, wl_resource* parent, int x, int y, unsigned int flags);
void shellSurfaceSetFullscreen(wl_client* client, wl_resource* resource, unsigned int method, unsigned int framerate, wl_resource* output);
void shellSurfaceSetPopup(wl_client* client, wl_resource* resource, wl_resource* seat, unsigned int serial, wl_resource* parent, int x, int y, unsigned int flags);
void shellSurfaceSetMaximized(wl_client* client, wl_resource* resource, wl_resource* output);
void shellSurfaceSetTitle(wl_client* client, wl_resource* resource, const char* title);
void shellSurfaceSetClass(wl_client* client, wl_resource* resource, const char* class_);

//dataOffer
void dataOfferAccept(wl_client* client, wl_resource* resource, unsigned int serial, const char* mime_type);
void dataOfferReceive(wl_client* client, wl_resource* resource, const char* mime_type, int fd);
void dataOfferDestroy(wl_client* client, wl_resource* resource);

//dataSource
void dataSourceOffer(wl_client* client, wl_resource* resource, const char* mime_type);
void dataSourceDestroy(wl_client* client, wl_resource* resource);

//dataDevice
void dataDeviceStartDrag(wl_client* client, wl_resource* resource, wl_resource* source, wl_resource* origin, wl_resource* icon, unsigned int serial);
void dataSeivceSetSelection(wl_client* client, wl_resource* resource, wl_resource* source, unsigned int serial);
void dataDeviceRelease(wl_client* client, wl_resource* resource);

//dataDeviceManager
void dataDeviceManagerCreateDataSource(wl_client* client, wl_resource* resource, unsigned int id);
void dataDeviceManagerGetDataDevice(wl_client* client, wl_resource* resource, unsigned int id, wl_resource* seat);




//interfaces
//display
const struct wl_display_interface displayImplementation =
{
    &displaySync,
    &displayGetRegistry
};

//registry
const struct wl_registry_interface registryImplementation =
{
    &registryBind
};

//compositor
const struct wl_compositor_interface compositorImplementation =
{
    &compositorCreateSurface,
    &compositorCreateRegion
};

//subcompositor
const struct wl_subcompositor_interface subcompositorImplementation =
{
    &subcompositorDestroy,
    &subcompositorGetSubsurface
};

//shell
const struct wl_shell_interface shellImplementation =
{
    &shellGetShellSurface
};

//surface
const struct wl_surface_interface surfaceImplementation =
{
    &surfaceDestroy,
    &surfaceAttach,
    &surfaceDamage,
    &surfaceFrame,
    &surfaceOpaqueRegion,
    &surfaceInputRegion,
    &surfaceCommit,
    &surfaceBufferTransform,
    &surfaceBufferScale
};

//region
const struct wl_region_interface regionImplementation
{
    &regionDestroy,
    &regionAdd,
    &regionSubtract
};

//shellSurface
const struct wl_shell_surface_interface shellSurfaceImplementation
{
    &shellSurfacePong,
    &shellSurfaceMove,
    &shellSurfaceResize,
    &shellSurfaceSetToplevel,
    &shellSurfaceSetTransient,
    &shellSurfaceSetFullscreen,
    &shellSurfaceSetPopup,
    &shellSurfaceSetMaximized,
    &shellSurfaceSetTitle,
    &shellSurfaceSetClass
};

//dataOffer
const struct wl_data_offer_interface dataOfferImplementation
{
    &dataOfferAccept,
    &dataOfferReceive,
    &dataOfferDestroy
};

//dataSource
const struct wl_data_source_interface dataSourceImplementation
{
    &dataSourceOffer,
    &dataSourceDestroy
};

//dataDevice
const struct wl_data_device_interface dataDeviceImplementation
{
    &dataDeviceStartDrag,
    &dataSeivceSetSelection,
    &dataDeviceRelease
};

//dataDeviceManager
const struct wl_data_device_manager_interface dataDeviceManagerImplementation
{
    &dataDeviceManagerCreateDataSource,
    &dataDeviceManagerGetDataDevice
};


} //wlInterface
