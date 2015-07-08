#include "server.hpp"
#include "wlInterface.hpp"
#include "x11Backend.hpp"

#include <wayland-server-protocol.h>
#include <string>
#include <iostream>

#include <utils/vec.hpp>
using namespace utils;

server* server::iro = 0;

server::server()
{
}

bool server::init()
{
    if(iro)
    {
        return 0;
    }

    if(!(display_ = wl_display_create()))
    {
        return 0;
    }

    if(wl_display_init_shm(display_) != 0)
    {
        return 0;
    }


    std::string sock = wl_display_add_socket_auto(display_);
    std::cout << "on: " << sock << std::endl;

    using namespace wlInterface;

    if(!wl_global_create(display_, &wl_compositor_interface, 3, this, bindCompositor))
    {
        return 0;
    }

    if(!wl_global_create(display_, &wl_subcompositor_interface, 1, this, bindSubcompositor))
    {
        return 0;
    }

    if(!wl_global_create(display_, &wl_shell_interface, 1, this, bindShell))
    {
        return 0;
    }

    iro = this;

    backend_ = new x11Backend();
    if(!backend_->init(vec2ui(800, 500)))
    {
        iro = 0;
        return 0;
    }

    return 1;
}

void server::run()
{
    wl_display_run(display_);
}

wl_event_loop* server::getWlEventLoop() const
{
    return wl_display_get_event_loop(display_);
}


///////////////
server* getServer()
{
    return server::getServer();
}
