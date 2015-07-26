CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(iro)

set(IRO_VMAJOR 0)
set(IRO_VMINOR 1)

include(ExternalProject)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/src")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMake)

#config files
#config.h
configure_file	( 
	"${CMAKE_CURRENT_SOURCE_DIR}/src/iro/config.h.in"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/iro/config.h"
 	@ONLY)

#pkg-config
configure_file	( 
	"${CMAKE_CURRENT_SOURCE_DIR}/iro.pc.in"
	"${CMAKE_CURRENT_BINARY_DIR}/iro.pc"
 	@ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/iro.pc" DESTINATION lib/pkgconfig)

#uninstall
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/CMake/uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/CMake/uninstall.cmake"
	IMMEDIATE @ONLY)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/CMake/uninstall.cmake)


#nyutil
find_package(nyutil)

if(NYUTIL_FOUND)
	set(nyutil_lib ${NYUTIL_LIBRARIES})
	set(nyutil_include ${NYUTIL_INCLUDE_DIR})

else(NYUTIL_FOUND)
	set(nyutil_prefix "${CMAKE_CURRENT_BINARY_DIR}/nyutil")
	set(nyutil_args "-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")

	ExternalProject_Add (
		nyutilTarget
		PREFIX ${nyutil_prefix}
		GIT_REPOSITORY https://github.com/nyorain/nyutil
		CMAKE_ARGS ${nyutil_args}
		INSTALL_COMMAND echo install_command_nyutil
	)

	set(nyutil_lib ${nyutil_prefix}/src/nyutilTarget-build/src/libnyutil.so)
	set(nyutil_include ${nyutil_prefix}/src/nyutilTarget/src)

	#install nyutil with iro
	install(DIRECTORY ${nyutil_prefix}/src/nyutilTarget/src/nyutil DESTINATION include)
	install(FILES ${nyutil_prefix}/src/nyutilTarget-build/nyutil.pc DESTINATION lib/pkgconfig)
	install(FILES ${nyutil_prefix}/src/nyutilTarget-build/src/libnyutil.so DESTINATION lib)
endif()


#subdirs
add_subdirectory(src/iro)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/iro" DESTINATION include/iro)

