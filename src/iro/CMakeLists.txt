#sources
set (CMAKE_POSITION_INDEPENDENT_CODE TRUE)

set(src
	compositor/compositor.cpp
	compositor/subcompositor.cpp
	compositor/callback.cpp
	compositor/region.cpp
	compositor/resource.cpp
	compositor/surface.cpp
	compositor/subsurface.cpp
	compositor/buffer.cpp
	compositor/shellSurface.cpp
	compositor/client.cpp
	compositor/xdg-surface.cpp
	compositor/shell.cpp
	compositor/xdg-shell.cpp

	backend/egl.cpp
	backend/tty.cpp
	backend/backend.cpp
	backend/renderer.cpp
	backend/output.cpp
	backend/session.cpp
	backend/x11Backend.cpp
	backend/kmsBackend.cpp
	backend/input.cpp

	util/shader.cpp
	util/iroModule.cpp

	seat/seat.cpp
	seat/keyboard.cpp
	seat/pointer.cpp

	iro.cpp
)

#deps
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

include_directories( 
	${nyutil_include}
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

#proto
add_subdirectory(proto)

#iro
add_library(iro SHARED ${src})
target_compile_features(iro PRIVATE cxx_range_for)

target_link_libraries(iro
	${nyutil_lib}
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

)

if(NOT ${NYUTIL_FOUND})
	add_dependencies(iro nyutilTarget)
endif()

install(TARGETS iro DESTINATION lib)

#main
add_executable(iroCompositor main.cpp)
target_compile_features(iroCompositor PRIVATE cxx_range_for)
target_link_libraries(iroCompositor iro ${nyutil_lib})

if(NOT ${NYUTIL_FOUND})
	add_dependencies(iroCompositor nyutilTarget)
endif()

install(TARGETS iroCompositor DESTINATION bin)




