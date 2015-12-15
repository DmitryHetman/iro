set(desktop-src
    main.cpp
)

add_executable(iro-desktop ${desktop-src})
target_link_libraries(iro-desktop iro)

target_compile_features(iro-desktop PRIVATE cxx_range_for)
