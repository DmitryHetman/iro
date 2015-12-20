set(shell-src
	shell.cpp
)

add_library(iro-shell MODULE ${shell-src})
target_compile_features(iro-shell PRIVATE cxx_range_for)

