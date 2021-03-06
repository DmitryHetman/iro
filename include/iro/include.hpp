#pragma once

#include "config.h"

//hide deprecated wayland functions
#define WL_HIDE_DEPRECATED

//wayland
struct wl_display;
struct wl_event_loop;
struct wl_global;
struct wl_resource;
struct wl_client;
struct wl_interface;
struct wl_event_source;
struct wl_output;
struct wl_shm_buffer;
struct wl_listener;
struct wl_array;

//nytl predefs
namespace nytl
{

template <typename T>
class ObservingPtr;

}

//iro predefs
namespace iro
{

//iro
class Compositor;
class Seat;
class Pointer;
class Keyboard;
class Touch;
class Renderer;
class Backend;
class Output;
class Client;
class Event;
class Subcompositor;
class Window;
class ShellModule;

class SurfaceContext;

class XWindowManager;

class InputHandler;
class TerminalHandler;
class LogindHandler;
class DBusHandler;
class UDevHandler;
class ForkHandler;
class DeviceHandler;
class Device;
class Fork;

class Resource;
class SurfaceRes;
class SubsurfaceRes;
class RegionRes;
class CallbackRes;
class BufferRes;
class CompositorRes;
class SubcompositorRes;
class ShellRes;
class SeatRes;
class PointerRes;
class KeyboardRes;
class TouchRes;
class OutputRes;

class WaylandEglContext;

template<typename R>
using ResourcePtr = nytl::ObservingPtr<R>;

using SurfacePtr = ResourcePtr<SurfaceRes>;
using BufferPtr = ResourcePtr<BufferRes>;
using CallbackPtr = ResourcePtr<CallbackRes>;
using PointerPtr = ResourcePtr<PointerRes>;
using KeyboardPtr = ResourcePtr<KeyboardRes>;

}

