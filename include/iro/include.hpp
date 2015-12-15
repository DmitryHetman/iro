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

//nytl predefs
namespace nytl
{

template <typename T>
class watchableRef;

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

class SurfaceContext;

class InputHandler;
class TerminalHandler;
class LogindHandler;
class DBusHandler;
class DeviceHandler;
class Device;

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
using ResourceRef = nytl::watchableRef<R>;

using SurfaceRef = ResourceRef<SurfaceRes>;
using BufferRef = ResourceRef<BufferRes>;
using CallbackRef = ResourceRef<CallbackRes>;
using PointerRef = ResourceRef<PointerRes>;
using KeyboardRef = ResourceRef<KeyboardRes>;

}

