#pragma once

#define WL_HIDE_DEPRECATED

namespace util {};
using namespace util;

struct wl_display;
struct wl_event_loop;
struct wl_global;
struct wl_resource;
struct wl_client;
struct wl_interface;
struct wl_event_source;
struct wl_output;
struct wl_shm_buffer;


class server;
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
class shader;
class texProgram;

class resource;
class surfaceRes;
class shellSurfaceRes;
class subsurfaceRes;
class regionRes;

class eglContext;
class renderer;

class x11Backend;
class x11Output;

class kmsBackend;
class kmsOutput;

class ttyHandler;
class inputHandler;
class sessionManager;

class device;

class event;
class pointerButtonEvent;
class pointerFocusEvent;
class keyboardKeyEvent;
class keyboardFocusEvent;

class iroModule;
class iroShell;
class iroShellExtension;


enum class backendType : unsigned char;
enum class bufferFormat : unsigned char;
enum class bufferType : unsigned char;
enum class resourceType : unsigned char;

iro* getIro();
compositor* iroCompositor();
sessionManager* iroSessionManager();
subcompositor* getSubcompositor();
backend* iroBackend();
seat* iroSeat();
pointer* iroPointer();
keyboard* iroKeyboard();
shell* iroShell();
eglContext* iroEglContext();
ttyHandler* iroTTYHandler();
inputHandler* iroInputHandler();

wl_display* iroWlDisplay();
wl_event_loop* iroWlEventLoop();

unsigned int iroNextSerial();
event* iroGetEvent(unsigned int serial);
void iroRegisterEvent(event& ev);

unsigned int iroTime();

//todo
class dataSource;
class dataOffer;
class dataDeviceManager;
class dataDevice;
