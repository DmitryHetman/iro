CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(iro VERSION 0.1.0)

set(CMAKE_BUILD_TYPE Debug)

set(IRO_VMAJOR 0)
set(IRO_VMINOR 1)
set(IRO_VPATCH 0)

#rpath
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)

#include dirs
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/src")

link_directories("${CMAKE_CURRENT_BINARY_DIR}/src/iro-shell")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

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
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/cmake/uninstall.cmake"
	IMMEDIATE @ONLY)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P 
	${CMAKE_CURRENT_BINARY_DIR}/cmake/uninstall.cmake)


#subdirs
add_subdirectory(src/iro)
add_subdirectory(src/iro-shell)
add_subdirectory(src/iro-desktop)
add_subdirectory(src/protos)
add_subdirectory(doc)
