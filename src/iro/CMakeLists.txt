#deps
find_package(NYTL REQUIRED)
find_package(Wayland REQUIRED)
find_package(libinput REQUIRED)
find_package(X11 REQUIRED)
find_package(XCB REQUIRED)
find_package(EGL REQUIRED)
find_package(DRM REQUIRED)
find_package(UDEV REQUIRED)
find_package(GLESv2 REQUIRED)
find_package(Dbus REQUIRED)
find_package(Systemd REQUIRED)
find_package(X11_XCB REQUIRED)
find_package(PAM REQUIRED)

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

	backend/egl.cpp
	backend/backend.cpp
	backend/output.cpp
	backend/x11.cpp
	backend/kms.cpp
	backend/surfaceContext.cpp
	backend/devices.cpp
	backend/tty.cpp
	#backend/logind.hpp
	#backend/dbus.hpp

	util/global.cpp

	seat/seat.cpp
	seat/keyboard.cpp
	seat/pointer.cpp
	seat/touch.cpp
)

#iro
add_library(iro SHARED ${iro-src})
target_compile_features(iro PRIVATE cxx_range_for)


target_include_directories(iro PUBLIC
	${NYTL_INCLUDE_DIRS}
	${WAYLAND_SERVER_INCLUDE_DIRS}
	${GBM_INCLUDE_DIRS}
	${DRM_INCLUDE_DIRS}
	${EGL_INCLUDE_DIRS}
	${GLESv2_INCLUDE_DIRS}
	${UDEV_INCLUDE_DIRS}
	${LIBINPUT_INCLUDE_DIRS}
	${XCB_INCLUDE_DIRS}
	${X11_INCLUDE_DIR}
	${DBUS_INCLUDE_DIRS}
	${SYSTEMD_INCLUDE_DIR}
	${X11_XCB_INCLUDE_DIR}
	${XCB_ICCCM_INCLUDE_DIR}
	${PAM_INCLUDE_DIR}
)

target_link_libraries(iro
	${WAYLAND_SERVER_LIBRARIES}
  	${LIBINPUT_LIBRARIES}
   	${UDEV_LIBRARIES}
	${GLESv2_LIBRARIES}
	${EGL_LIBRARIES}
	${XCB_LIBRARIES}
	${X11_LIBRARIES}
	${DBUS_LIBRARIES}
	${GBM_LIBRARIES}
	${SYSTEMD_LIBRARIES}
	${DRM_LIBRARIES}
	${X11_XCB_LIBRARIES}
	${XCB_ICCCM_LIBRARY}
	${PAM_LIBRARIES}
	ny-draw
)
