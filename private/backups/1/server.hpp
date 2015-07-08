#pragma once

struct wl_display;
struct wl_event_loop;

class x11Backend;
class surface;


class server
{
protected:
    static server* iro;

    wl_display* display_ = 0;
    x11Backend* backend_ = 0;

public:
    server();
    bool init();

    void run();

    wl_display* getWlDisplay() const { return display_; }
    wl_event_loop* getWlEventLoop() const;

    x11Backend* getBackend() const { return backend_; }

    static server* getServer() { return iro; }
};

server* getServer();
