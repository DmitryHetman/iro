#deps
find_package(NYTL REQUIRED)
find_package(Wayland REQUIRED)
find_package(libinput REQUIRED)
find_package(X11 REQUIRED)
find_package(XCB REQUIRED COMPONENTS ewmh composite xkb image icccm)
find_package(EGL REQUIRED)
find_package(DRM REQUIRED)
find_package(UDEV REQUIRED)
find_package(Dbus REQUIRED)
find_package(Systemd REQUIRED)
find_package(X11_XCB REQUIRED)
find_package(XKBCommon REQUIRED)

MESSAGE(STATUS "dir: " ${WAYLAND_SERVER_LIBRARIES})

set(iro-src
	compositor/compositor.cpp
	compositor/callback.cpp
	compositor/region.cpp
	compositor/resource.cpp
	compositor/surface.cpp
	compositor/subsurface.cpp
	compositor/subcompositor.cpp
	compositor/buffer.cpp
	compositor/client.cpp
	compositor/shell.cpp
	compositor/shellSurface.cpp
	compositor/xdgShell.cpp
	compositor/xdgSurface.cpp
	compositor/xdgPopup.cpp
	compositor/window.cpp

	backend/egl.cpp
	backend/backend.cpp
	backend/output.cpp
	backend/x11.cpp
	backend/kms.cpp
	backend/surfaceContext.cpp
	backend/devices.cpp
	backend/tty.cpp
	backend/input.cpp
	backend/udev.cpp
	backend/logind.cpp
	backend/dbus.cpp

	util/global.cpp
	util/os.cpp
	util/fork.cpp

	seat/seat.cpp
	seat/keyboard.cpp
	seat/pointer.cpp
	seat/touch.cpp
	seat/cursorSurface.cpp

	xwayland/xwm.cpp
	xwayland/xwindow.cpp
)

#iro
add_library(iro SHARED ${iro-src})

target_include_directories(iro PUBLIC
	${NYTL_INCLUDE_DIRS}
	${WAYLAND_SERVER_INCLUDE_DIRS}
	${GBM_INCLUDE_DIRS}
	${DRM_INCLUDE_DIRS}
	${EGL_INCLUDE_DIRS}
	${UDEV_INCLUDE_DIRS}
	${LIBINPUT_INCLUDE_DIRS}
	${XCB_INCLUDE_DIRS}
	${X11_INCLUDE_DIRS}
	${DBUS_INCLUDE_DIRS}
	${SYSTEMD_INCLUDE_DIRS}
	${X11_XCB_INCLUDE_DIRS}
	${XCB_ICCCM_INCLUDE_DIRS}
	${XKBCOMMON_LUDE_DIRS}
)

target_link_libraries(iro
	${WAYLAND_SERVER_LIBRARIES}
	#"libwayland-server.so"
  	${LIBINPUT_LIBRARIES}
   	${UDEV_LIBRARIES}
	${EGL_LIBRARIES}
	${XCB_LIBRARIES}
	${X11_LIBRARIES}
	${DBUS_LIBRARIES}
	${GBM_LIBRARIES}
	${SYSTEMD_LIBRARIES}
	${DRM_LIBRARIES}
	${X11_XCB_LIBRARIES}
	${XCB_ICCCM_LIBRARIEs}
	${XKBCOMMON_LIBRARIES}
	${CMAKE_DL_LIBS}
	/usr/lib/libny-draw.so
	/usr/lib/libny-base.so
	iro-protos
)
