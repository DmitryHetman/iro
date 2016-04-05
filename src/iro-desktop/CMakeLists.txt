set(desktop-src
    main.cpp
)

add_executable(iro-desktop ${desktop-src})
target_link_libraries(iro-desktop iro)
