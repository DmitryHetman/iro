#pragma once

#define WL_HIDE_DEPRECATED

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

enum class backendType : unsigned char;
enum class bufferFormat : unsigned char;
enum class bufferType : unsigned char;

server* getServer();
compositor* getCompositor();
subcompositor* getSubcompositor();
backend* getBackend();
seat* getSeat();
shell* getShell();
eglContext* getEglContext();

ttyHandler* getTTYHandler();
inputHandler* getInputHandler();

void addClientResource(wl_client* c, resource* res);

wl_display* getWlDisplay();
wl_event_loop* getWlEventLoop();

unsigned int getTime();

//todo
class dataSource;
class dataOffer;
class dataDeviceManager;
class dataDevice;
