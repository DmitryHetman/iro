#login
add_library(iro-login-shell MODULE login/login-module.cpp)
target_link_libraries(iro-login-shell iro ${nyutil_libraries})
target_compile_features(iro-login-shell PRIVATE cxx_range_for)

install(TARGETS iro-login-shell DESTINATION lib)

#desktop
add_library(iro-desktop-shell MODULE desktop/desktop-module.cpp)
target_link_libraries(iro-desktop-shell iro ${nyutil_libraries})
target_compile_features(iro-desktop-shell PRIVATE cxx_range_for)

install(TARGETS iro-desktop-shell DESTINATION lib)
