CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(iro)

set(IRO_VMAJOR 0)
set(IRO_VMINOR 1)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMake)

add_subdirectory(src)

# uninstall target
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/CMake/uninstall.cmake.in"
	"${CMAKE_CURRENT_SOURCE_DIR}/CMake/uninstall.cmake"
	IMMEDIATE @ONLY)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/CMake/uninstall.cmake)

SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE) 

