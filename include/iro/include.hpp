#pragma once

#include "config.h"

#define WL_HIDE_DEPRECATED

namespace nyutil {};
using namespace nyutil;

struct wl_display;
struct wl_event_loop;
struct wl_global;
struct wl_resource;
struct wl_client;
struct wl_interface;
struct wl_event_source;
struct wl_output;
struct wl_shm_buffer;


class iro;
class client;
class compositor;
class subcompositor;
class shell;
class seat;

class pointer;
class keyboard;

class compositorRes;
class subcompositorRes;
class shellRes;
class seatRes;
class outputRes;
class bufferRes;

class callbackRes;

class pointerRes;
class keyboardRes;

class backend;
class output;

class renderer;
class texProgram;

class resource;
class surfaceRes;
class shellSurfaceRes;
class subsurfaceRes;
class regionRes;

template <typename> class resourceRef;
typedef resourceRef<surfaceRes> surfaceRef;
typedef resourceRef<bufferRes> bufferRef;
typedef resourceRef<callbackRes> callbackRef;
typedef resourceRef<pointerRes> pointerRef;

class eglContext;
class renderer;
class glRenderer;
class renderData;

class x11Backend;
class x11Output;

class kmsBackend;
class kmsOutput;

class inputHandler;
class sessionManager;
class deviceManager;

class device;

class event;
class pointerButtonEvent;
class pointerFocusEvent;
class keyboardKeyEvent;
class keyboardFocusEvent;

class iroModule;
class iroShellModule;
class iroShellExtension;

class iroDrawContext;

enum class resourceType : unsigned char;

iro* getIro();
compositor* iroCompositor();
subcompositor* getSubcompositor();
backend* iroBackend();
sessionManager* iroSessionManager();
seat* iroSeat();
pointer* iroPointer();
keyboard* iroKeyboard();
eglContext* iroEglContext();
renderer* iroRenderer();
iroShellModule* iroShell();

wl_display* iroWlDisplay();
wl_event_loop* iroWlEventLoop();

unsigned int iroNextSerial(event* ev);
event* iroGetEvent(unsigned int serial);
void iroRegisterEvent(event& ev);

unsigned int iroTime();

//todo
class dataSource;
class dataOffer;
class dataDeviceManager;
class dataDevice;
