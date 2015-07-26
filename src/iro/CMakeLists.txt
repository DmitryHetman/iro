#sources
set(lib_src
	compositor/compositor.cpp
	compositor/subcompositor.cpp

	backend/egl.cpp
	backend/tty.cpp
	backend/backend.cpp
	backend/renderer.cpp
	backend/output.cpp
	backend/session.cpp
	backend/x11/x11Backend.cpp
	backend/kms/kmsBackend.cpp
	backend/kms/input.cpp

	shell/shell.cpp
	shell/xdg-shell.cpp

	shader/shader.cpp

	resources/callback.cpp
	resources/region.cpp
	resources/resource.cpp
	resources/surface.cpp
	resources/subsurface.cpp
	resources/buffer.cpp
	resources/shellSurface.cpp
	resources/client.cpp
	resources/xdg-surface.cpp

	seat/seat.cpp
	seat/keyboard.cpp
	seat/pointer.cpp

	iro.cpp
)

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

include_directories( 
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
)

include(Wayland)
wayland_add_protocol_server(proto-xdg-shell "${CMAKE_SOURCE_DIR}/proto/xdg-shell.xml" xdg-shell)
list(APPEND src ${proto-xdg-shell})

add_library(iro ${src})
target_compile_features(iro PRIVATE cxx_range_for)

target_link_libraries(iro
	~/Programming/linux/lib/libnyutil.so
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
)

add_executable(iro_prog main.cpp)
target_compile_features(iro_prog PRIVATE cxx_range_for)
target_link_libraries(iro_prog iro)




