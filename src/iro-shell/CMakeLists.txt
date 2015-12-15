set(shell-src
	shell.cpp
	#shellSurface.cpp
	#xdgShell.cpp
	#xdgShellSurface.cpp
)

#proto
add_subdirectory(proto)

add_library(iro-shell MODULE ${shell-src})
target_compile_features(iro-shell PRIVATE cxx_range_for)

target_link_libraries(iro-shell iro)
