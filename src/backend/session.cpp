#include <backend/session.hpp>
#include <log.hpp>

#include <wayland-server-core.h>

#include <systemd/sd-login.h>
#include <unistd.h>
#include <sys/eventfd.h>

#include <stdexcept>

int dbusDispatch(int fd, uint32_t mask, void* data)
{

}

//watch
dbus_bool_t dbusAddWatch(DBusWatch* watch, void* data)
{

}

void dbusRemoveWatch(DBusWatch* watch, void* data)
{

}

void dbusToggleWatch(DBusWatch* watch, void* data)
{

}

//timeout
dbus_bool_t dbusAddTimeout(DBusTimeout* timeout, void* data)
{

}

void dbusRemoveTimeout(DBusTimeout* timeout, void *data)
{

}

void dbusToggleTimeout(DBusTimeout* timeout, void* data)
{

}

//filter
DBusHandlerResult dbusFilter(DBusConnection* connection, DBusMessage* msg, void* data)
{

}

//////////////////////////////////////
//util

bool dbusAddMatchSignal(DBusConnection* conn, const std::string& sender, const std::string& iface, const std::string& member, const std::string& path)
{
    return false;
}

/////////////////////////////////////////
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
    sessionPath_ = "/org/freedesktop/login1/session/" + session_;

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

    //open dbus
    //error TODO: check for them
    DBusError err;
    dbus_error_init(&err);

    dbus_connection_set_change_sigpipe(false);

    if(!(dbus_ = dbus_bus_get_private(DBUS_BUS_SYSTEM, nullptr)))
    {
        throw std::runtime_error("cant connect to dbus");
        return;
    }

    dbus_connection_set_exit_on_disconnect(dbus_, false);

    //use dummy event fd
    int fd;
    if((fd = eventfd(0, EFD_CLOEXEC)) < 0)
    {
        throw std::runtime_error("cant create eventfd");
        return;
    }

    dbusEventSource_ = wl_event_loop_add_fd(iroWlEventLoop(), fd, 0, dbusDispatch, this);
    close(fd);
    wl_event_source_check(dbusEventSource_);

    if(!dbus_connection_set_watch_functions(dbus_, dbusAddWatch, dbusRemoveWatch, dbusToggleWatch, this, nullptr))
    {
        throw std::runtime_error("dbus_connection_set_watch_functions failed");
        return;
    }

    if(!dbus_connection_set_timeout_functions(dbus_, dbusAddTimeout, dbusRemoveTimeout, dbusToggleTimeout, this, nullptr))
    {
        throw std::runtime_error("dbus_connection_set_timeout_functions failed");
        return;
    }

    //setup dbus
    if(!dbus_connection_add_filter(dbus_, dbusFilter, this, nullptr) ||
       !dbusAddMatchSignal(dbus_, "org.freedesktop.login1", "org.freedesktop.login1.Manager", "SessionRemoved", "/org/freedesktop/login1") ||
       !dbusAddMatchSignal(dbus_, "org.freedesktop.login1", "org.freedesktop.login1.Session", "PauseDevice", sessionPath_) ||
       !dbusAddMatchSignal(dbus_, "org.freedesktop.login1", "org.freedesktop.login1.Session", "ResumeDevice", sessionPath_) ||
       !dbusAddMatchSignal(dbus_, "org.freedesktop.login1", "org.freedesktop.DBus.Properties", "PropertiesChanged", sessionPath_)
       )
    {
        throw std::runtime_error("dbus setup failed failed");
        return;
    }

    //take control over session
    DBusMessage* msg;
    if(!(msg = dbus_message_new_method_call("org.freedesktop.login1", sessionPath_.c_str(), "org.freedesktop.login1.Session", "TakeControl")))
    {
        throw std::runtime_error("call TakeControl failed");
        return;
    }

    dbus_bool_t arr[] = {false};
    if(!dbus_message_append_args(msg, DBUS_TYPE_BOOLEAN, arr, DBUS_TYPE_INVALID))
    {
        throw std::runtime_error("dbus_append_args for the take control msg failed");
        return;
    }

    DBusMessage* reply;
    if(!dbus_connection_send_with_reply_and_block(dbus_, msg, -1, &err))
    {
        if(dbus_error_is_set(&err))
        {
            dbus_error_free(&err);
        }

        throw std::runtime_error("dbus_connection_send_with_reply_and_block for the take control msg failed");
        return;
    }

    dbus_message_unref(reply);
    dbus_message_unref(msg);
}

sessionHandler::~sessionHandler()
{
    if(dbus_)
    {
        dbus_connection_close(dbus_);
        dbus_connection_unref(dbus_);

        dbus_ = nullptr;
    }

    if(dbusEventSource_)
    {
        wl_event_source_remove(dbusEventSource_);
        dbusEventSource_ = nullptr;
    }
}
