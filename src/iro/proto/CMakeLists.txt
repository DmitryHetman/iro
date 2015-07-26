include(Wayland)

#xdg-shell
wayland_add_protocol_server(proto-xdg-shell "${CMAKE_SOURCE_DIR}/proto/xdg-shell.xml" xdg-shell)
list(APPEND src ${proto-xdg-shell})
install(FILES wayland-xdg-shell-server-protocol.h DESTINATION include/iro/proto)
