#pragma once

#define WL_HIDE_DEPRECATED

class server;
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

class pointerRes;
class keyboardRes;

class backend;
class output;

class renderer;
class shader;
class surfaceProgram;

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

//todo
class dataSource;
class dataOffer;
class dataDeviceManager;
class dataDevice;
