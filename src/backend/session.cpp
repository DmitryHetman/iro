#include <backend/session.hpp>
#include <log.hpp>

#include <wayland-server-core.h>

#include <systemd/sd-login.h>
#include <unistd.h>
#include <stdexcept>

sessionHandler::sessionHandler()
{
    //systemd
    char* session = nullptr;
    if(sd_pid_get_session(getpid(), &session) < 0 || !session)
    {
        throw std::runtime_error("cant get sessionID from systemd");
        return;
    }
    session_ = session;
    free(session);

    char* seat = nullptr;
    if(sd_session_get_seat(session_.c_str(), &seat) < 0 || !seat)
    {
        throw std::runtime_error("cant get seat from systemd");
        return;
    }
    seat_ = seat;
    free(seat);

    if(sd_session_get_vt(session_.c_str(), &vt_) < 0)
    {
        throw std::runtime_error("cant get vt number from systemd");
        return;
    };

    iroLog("Running with systemd session ", session_, " and seat ", seat_, " on vt ", vt_);

    //dbus
    dbus_connection_set_change_sigpipe(false);

    if(!(dbus_ = dbus_bus_get_private(DBUS_BUS_SYSTEM, nullptr)))
    {
        throw std::runtime_error("cant connect to dbus");
        return;
    }

    dbus_connection_set_exit_on_disconnect(dbus_, false);
}

sessionHandler::~sessionHandler()
{
    dbus_connection_close(dbus_);
    dbus_connection_unref(dbus_);
}
