include(Wayland)

#xdg-shell
wayland_add_protocol_server(proto-xdg-shell "${CMAKE_CURRENT_SOURCE_DIR}/xdg-shell.xml" xdg-shell)
list(APPEND shell-src ${proto-xdg-shell})

#install
#install(FILES wayland-xdg-shell-server-protocol.h DESTINATION include/iro/proto)
