#include <backend/session.hpp>
#include <log.hpp>
#include <server.hpp>

#include <wayland-server-core.h>

#include <systemd/sd-login.h>
#include <unistd.h>
#include <sys/eventfd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>

#include <stdexcept>
#include <iostream>

//device
void device::release()
{
    iroSessionManager()->releaseDevice(*this);
}

//error
std::string checkDBusError(DBusError& err)
{
    if(dbus_error_is_set(&err))
    {
        std::string msg = "DBus Error " + std::string(err.name) + ": " + std::string(err.message);
        dbus_error_free(&err);
        return msg;
    }

    return std::string();
}

bool checkDBusError(DBusError& err, std::string& str)
{
    if(dbus_error_is_set(&err))
    {
        str = "DBus Error " + std::string(err.name) + ": " + std::string(err.message);
        dbus_error_free(&err);
        return 1;
    }

    return 0;
}

bool checkDBusErrorThrow(DBusError& err)
{
    std::string ret;
    if(checkDBusError(err, ret))
    {
        throw std::runtime_error(ret);
        return 1;
    }

    return 0;
}

bool checkDBusErrorWarn(DBusError& err)
{
    std::string ret;
    if(checkDBusError(err, ret))
    {
        iroWarning(ret);
        return 1;
    }

    return 0;
}

////
int dbusDispatch(int fd, uint32_t mask, void* data)
{
   sessionManager* sh = (sessionManager*)data;

   while(1)
   {
        switch(dbus_connection_dispatch(sh->getDBusConnection()))
        {
            case DBUS_DISPATCH_DATA_REMAINS: break;
            case DBUS_DISPATCH_COMPLETE: return 0;
            case DBUS_DISPATCH_NEED_MEMORY: iroWarning("DBUS_DISPATCH_NEED_MEMORY"); return 0;
            default: iroWarning("DBUS_DISPATCH_ERROR"); return 0;
        }
   }

   return 0;
}

//watch
int dispatchWatch(int fd, unsigned int mask, void* data)
{
    DBusWatch* watch = (DBusWatch*) data;

    if (dbus_watch_get_enabled(watch))
    {
        unsigned int flags = 0;

        if (mask & WL_EVENT_READABLE)
         flags |= DBUS_WATCH_READABLE;
        if (mask & WL_EVENT_WRITABLE)
         flags |= DBUS_WATCH_WRITABLE;
        if (mask & WL_EVENT_HANGUP)
         flags |= DBUS_WATCH_HANGUP;
        if (mask & WL_EVENT_ERROR)
         flags |= DBUS_WATCH_ERROR;

        dbus_watch_handle(watch, flags);
    }

    return 0;
}

//dbus cb
dbus_bool_t dbusAddWatch(DBusWatch* watch, void* data)
{
    unsigned int mask = 0;

    if (dbus_watch_get_enabled(watch))
    {
        unsigned int flags = dbus_watch_get_flags(watch);

        if (flags & DBUS_WATCH_READABLE) mask |= WL_EVENT_READABLE;
        if (flags & DBUS_WATCH_WRITABLE) mask |= WL_EVENT_WRITABLE;
    }

    int fd = dbus_watch_get_unix_fd(watch);
    wl_event_source* s;

    if (!(s = wl_event_loop_add_fd(iroWlEventLoop(), fd, mask, dispatchWatch, watch)))
        return false;

    dbus_watch_set_data(watch, s, nullptr);
    return true;
}

void dbusRemoveWatch(DBusWatch* watch, void* data)
{
   wl_event_source* s;
   if (!(s = (wl_event_source*) dbus_watch_get_data(watch)))
   {
        iroWarning("dbusRemoveWatch: dbus watch has no data");
        return;
   }

   wl_event_source_remove(s);
}

void dbusToggleWatch(DBusWatch* watch, void* data)
{
    struct wl_event_source *s;
    if(!(s = (wl_event_source*) dbus_watch_get_data(watch)))
    {
        iroWarning("dbusToggleWatch: dbus watch has no data");
        return;
    }

    uint32_t mask = 0;
    if(dbus_watch_get_enabled(watch))
    {
        unsigned int flags = dbus_watch_get_flags(watch);

        if (flags & DBUS_WATCH_READABLE) mask |= WL_EVENT_READABLE;
        if (flags & DBUS_WATCH_WRITABLE) mask |= WL_EVENT_WRITABLE;
    }

    wl_event_source_fd_update(s, mask);
}

//timeout
int dispatchTimeout(void* data)
{
   DBusTimeout* timeout = (DBusTimeout*) data;

   if (dbus_timeout_get_enabled(timeout))
      dbus_timeout_handle(timeout);

   return 0;
}

int adjustTimeout(DBusTimeout* timeout, wl_event_source* s)
{
   unsigned long iv = 0;
   if (dbus_timeout_get_enabled(timeout))
      iv = dbus_timeout_get_interval(timeout);

   return wl_event_source_timer_update(s, iv);
}

//dbus cb
dbus_bool_t dbusAddTimeout(DBusTimeout* timeout, void* data)
{
    wl_event_source* s;
    if(!(s = wl_event_loop_add_timer(iroWlEventLoop(), dispatchTimeout, timeout)))
    {
        return false;
    }

    if(adjustTimeout(timeout, s) < 0)
    {
        wl_event_source_remove(s);
        return false;
    }

    dbus_timeout_set_data(timeout, s, nullptr);
    return true;
}

void dbusRemoveTimeout(DBusTimeout* timeout, void *data)
{
    wl_event_source* s = (wl_event_source*) dbus_timeout_get_data(timeout);
    if(!s)
    {
        iroWarning("dbusRemoveTimeout: dbus timeout has no data");
        return;
    }

    wl_event_source_remove(s);
}

void dbusToggleTimeout(DBusTimeout* timeout, void* data)
{
    wl_event_source* s = (wl_event_source*) dbus_timeout_get_data(timeout);
    if(!s)
    {
        iroWarning("dbusToggleTimeout: dbus timeout has no data");
        return;
    }

    adjustTimeout(timeout, s);
}

//filter
void cbDisconnected(DBusMessage* msg)
{
    iroLog("Disconnected from dbus");
    iroServer()->exit();
}

void cbSessionRemoved(DBusMessage* msg)
{
    const char* name;
    const char* obj;
    if(!dbus_message_get_args(msg, nullptr, DBUS_TYPE_STRING, &name, DBUS_TYPE_OBJECT_PATH, &obj, DBUS_TYPE_INVALID) || std::string(name) != iroSessionManager()->getSession())
        return;

    iroLog("dbus session was removed");
    iroServer()->exit();
}

void cbPropertiesChanged(DBusMessage* m)
{
    DBusMessageIter iter;
    if (!dbus_message_iter_init(m, &iter) || dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING)
        goto error0;

    const char *interface;
    dbus_message_iter_get_basic(&iter, &interface);

    if (!dbus_message_iter_next(&iter) || dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
        goto error0;

    DBusMessageIter sub;
    dbus_message_iter_recurse(&iter, &sub);

    DBusMessageIter entry;
    while (dbus_message_iter_get_arg_type(&sub) == DBUS_TYPE_DICT_ENTRY)
    {
        dbus_message_iter_recurse(&sub, &entry);

        if(dbus_message_iter_get_arg_type(&entry) != DBUS_TYPE_STRING)
            goto error0;

        const char *name;
        dbus_message_iter_get_basic(&entry, &name);
        if(!dbus_message_iter_next(&entry))
            goto error0;

        if(std::string(name) == "Active")
        {
            //parse_active(m, &entry);
            return;
        }

        dbus_message_iter_next(&sub);
    }

    if (!dbus_message_iter_next(&iter) || dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
        goto error0;

    dbus_message_iter_recurse(&iter, &sub);

    while (dbus_message_iter_get_arg_type(&sub) == DBUS_TYPE_STRING)
    {
        const char *name;
        dbus_message_iter_get_basic(&sub, &name);

        if(std::string(name) == "Active")
        {
            //get_active();
            return;
        }

        dbus_message_iter_next(&sub);
    }

    return;

error0:
    iroLog("dbus: cannot parse PropertiesChanged dbus signal");
}

void cbDevicePaused(DBusMessage* msg)
{
    if(iroSessionManager())
        iroSessionManager()->devicePaused(msg);
}

void cbDeviceResumed(DBusMessage* msg)
{
    if(iroSessionManager())
        iroSessionManager()->deviceResumed(msg);
}

DBusHandlerResult dbusFilter(DBusConnection* connection, DBusMessage* msg, void* data)
{
    struct msgCB
    {
        const char* path;
        const char* name;
        void (*func)(DBusMessage* m);
    };

    msgCB map[] =
    {
        { DBUS_INTERFACE_LOCAL, "Disconnected", cbDisconnected },
        { "org.freedesktop.login1.Manager", "SessionRemoved", cbSessionRemoved },
        { "org.freedesktop.DBus.Properties", "PropertiesChanged", cbPropertiesChanged },
        { "org.freedesktop.login1.Session", "PauseDevice", cbDevicePaused },
        { "org.freedesktop.login1.Session", "ResumeDevice", cbDeviceResumed }
    };

    for(auto& cb : map)
    {
        if(dbus_message_is_signal(msg, cb.path, cb.name))
        {
            cb.func(msg);
            break;
        }
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

//////////////////////////////////////
//util
bool dbusAddMatch(DBusConnection* conn, const std::string& match)
{
    DBusError err;
    dbus_error_init(&err);

    dbus_bus_add_match(conn, match.c_str(), &err);

    return !checkDBusErrorWarn(err);
}

bool dbusAddMatchSignal(DBusConnection* conn, const std::string& sender, const std::string& interface, const std::string& member, const std::string& path)
{
    return dbusAddMatch(conn, "type='signal',sender='" + sender + "',interface='" + interface + "',member='" + member + "',path='" + path + "'");
}

/////////////////////////////////////////
sessionManager::sessionManager()
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

    std::string errStr;
    if(!(dbus_ = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err)) || checkDBusError(err, errStr))
    {
        throw std::runtime_error("cant connect to dbus. " + (errStr.empty()) ? "" : errStr);
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

    DBusMessage* reply = nullptr;
    if(!(reply = dbus_connection_send_with_reply_and_block(dbus_, msg, -1, &err)) || checkDBusError(err, errStr) || !reply)
    {
        throw std::runtime_error("dbus_connection_send_with_reply_and_block for the take control msg failed. " + (errStr.empty()) ? "" : errStr);
        return;
    }

    dbus_message_unref(reply);
    dbus_message_unref(msg);

    iroLog("Took Session Control");
}

sessionManager::~sessionManager()
{
    if(dbus_)
    {
        //release control
        DBusMessage* m;
        if (!(m = dbus_message_new_method_call("org.freedesktop.login1", sessionPath_.c_str(), "org.freedesktop.login1.Session", "ReleaseControl")))
        {
            iroLog("sessionManager::~sessionManager: dbus_message_new_method_call for release control failed");
        }

        dbus_connection_send(dbus_, m, nullptr);
        dbus_message_unref(m);

        //cleanup dbus
        dbus_connection_set_timeout_functions(dbus_, nullptr, nullptr, nullptr, nullptr, nullptr);
        dbus_connection_set_watch_functions(dbus_, nullptr, nullptr, nullptr, nullptr, nullptr);

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

device* sessionManager::takeDevice(const std::string& path)
{
    device* ret = new device;

    ret->path = path;
    ret->active = 0;
    ret->fd = -1;

    struct stat st;
    if(stat(path.c_str(), &st) < 0 || !S_ISCHR(st.st_mode))
    {
        iroWarning("sessionManager::takeDevice: failed to get stat struct for path");
        return nullptr;
    }

    unsigned int majr = major(st.st_rdev);
    unsigned int minr = minor(st.st_rdev);

    DBusMessage* msg;
    if(!(msg = dbus_message_new_method_call("org.freedesktop.login1", sessionPath_.c_str(), "org.freedesktop.login1.Session", "TakeDevice")))
    {
        iroWarning("sessionManager::takeDevice: dbus_message_new_method_call failed");
        dbus_message_unref(msg);
        return nullptr;
    }

    if(!dbus_message_append_args(msg, DBUS_TYPE_UINT32, &majr, DBUS_TYPE_UINT32, &minr, DBUS_TYPE_INVALID))
    {
        iroWarning("sessionManager::takeDevice: dbus_message_append_args failed");
        dbus_message_unref(msg);
        return nullptr;
    }

    DBusMessage *reply;
    if(!(reply = dbus_connection_send_with_reply_and_block(dbus_, msg, -1, nullptr)))
    {
        iroWarning("sessionManager::takeDevice: dbus_connection_send_with_reply_and_block failed");
        dbus_message_unref(msg);
        return nullptr;
    }

    int fd;
    dbus_bool_t paused;
    if(!dbus_message_get_args(reply, nullptr, DBUS_TYPE_UNIX_FD, &fd, DBUS_TYPE_BOOLEAN, &paused, DBUS_TYPE_INVALID))
    {
        iroWarning("sessionManager::takeDevice: dbus_message_get_args failed");
        dbus_message_unref(reply);
        dbus_message_unref(msg);
        return nullptr;
    }

    int fl;
    if((fl = fcntl(fd, F_GETFL)) < 0 || fcntl(fd, F_SETFD, fl | FD_CLOEXEC) < 0)
    {
        iroWarning("sessionManager::takeDevice: invalid fd");
        close(fd);
        dbus_message_unref(reply);
        dbus_message_unref(msg);
        return nullptr;
    }

    dbus_message_unref(reply);
    dbus_message_unref(msg);

    ret->active = !paused;
    ret->fd = fd;

    devices_.push_back(ret);

    return ret;
}

void sessionManager::releaseDevice(device& dev)
{
    releaseDevice(dev.fd);
}

void sessionManager::releaseDevice(int devFD)
{
    struct stat st;
    if(fstat(devFD, &st) < 0 || !S_ISCHR(st.st_mode))
    {
        iroWarning("sessionManager::releaseDevice: failed to get stat struct for devFD");
        return;
    }

    unsigned int majr = major(st.st_rdev);
    unsigned int minr = minor(st.st_rdev);

    bool found = 0;
    for(auto it = devices_.begin(); it != devices_.end(); ++it)
    {
        if(devFD == (*it)->fd)
        {
            found = 1;

            //dbus release
            DBusMessage* msg;
            if (!(msg = dbus_message_new_method_call("org.freedesktop.login1", sessionPath_.c_str(), "org.freedesktop.login1.Session", "ReleaseDevice")))
            {
                iroWarning("sessionManager::releaseDevice: dbus_message_new_method_call failed");
                dbus_message_unref(msg);
                return;
            }

            if(!dbus_message_append_args(msg, DBUS_TYPE_UINT32, &majr, DBUS_TYPE_UINT32, &minr, DBUS_TYPE_INVALID))
            {
                iroWarning("sessionManager::releaseDevice: dbus_message_append_args failed");
                dbus_message_unref(msg);
                return;
            }

            dbus_connection_send(dbus_, msg, nullptr);
            dbus_message_unref(msg);

            //delete/erase
            delete *it;
            devices_.erase(it);
            break;
        }
    }

    if(!found) iroWarning("sessionManager::releaseDevice: device not found");
}

void sessionManager::devicePaused(DBusMessage* msg)
{
    const char* type;
    unsigned int majr, minr;

    if(!dbus_message_get_args(msg, nullptr, DBUS_TYPE_UINT32, &majr, DBUS_TYPE_UINT32, &minr, DBUS_TYPE_STRING, &type, DBUS_TYPE_INVALID))
    {
        iroWarning("sessionManager::devicePaused: dbus_message_get_args failed");
        return;
    }

    if(std::string(type) == "pause")
    {
        //pause it
        DBusMessage* m;
        if (!(m = dbus_message_new_method_call("org.freedesktop.login1", sessionPath_.c_str(), "org.freedesktop.login1.Session", "PauseDeviceComplete")))
        {
            iroWarning("sessionManager::devicePaused: dbus_message_new_method_call failed");
            return;
        }


        if (!dbus_message_append_args(m, DBUS_TYPE_UINT32, &majr, DBUS_TYPE_UINT32, &minr, DBUS_TYPE_INVALID))
        {
            iroWarning("sessionManager::devicePaused: dbus_message_append_args failed");
            return;
        }

        dbus_connection_send(dbus_, m, nullptr);
        dbus_message_unref(m);
    }

    struct stat st;
    bool found = 0;
    for(auto* dev : devices_)
    {
        if(fstat(dev->fd, &st) < 0 || !S_ISCHR(st.st_mode))
        {
            iroWarning("sessionManager::devicePaused: failed to get stat struct for device fd");
            continue;
        }

        unsigned int fdMajor = major(st.st_rdev);
        unsigned int fdMinor = minor(st.st_rdev);

        if(fdMajor == majr && fdMinor == minr)
        {
            dev->active = 0;
            dev->pauseCallback_(*dev);
            found = 1;
        }
    }

    if(!found) iroWarning("sessionManager::devicePaused: device not found");
}

void sessionManager::deviceResumed(DBusMessage* msg)
{
    unsigned int majr, minr;
    if(!dbus_message_get_args(msg, nullptr, DBUS_TYPE_UINT32, &majr, DBUS_TYPE_UINT32, &minr, DBUS_TYPE_INVALID))
    {
        iroWarning("sessionManager::deviceResumed: dbus_message_get_args failed");
        return;
    }

    struct stat st;
    bool found = 0;
    for(auto* dev : devices_)
    {
        if(fstat(dev->fd, &st) < 0 || !S_ISCHR(st.st_mode))
        {
            iroWarning("sessionManager::deviceResumed: failed to get stat struct for device fd");
            continue;
        }

        unsigned int fdMajor = major(st.st_rdev);
        unsigned int fdMinor = minor(st.st_rdev);

        if(fdMajor == majr && fdMinor == minr)
        {
            dev->active = 1;
            dev->resumeCallback_(*dev);
            found = 1;
        }
    }

    if(!found) iroWarning("sessionManager::deviceResumed: device not found");
}


//pam
int pamConversation(int msg_count, const struct pam_message** messages, struct pam_response** responses, void* data)
{
	return PAM_SUCCESS;
}

pamHandler::pamHandler()
{
    pamConv_.conv = &pamConversation;
    pamConv_.appdata_ptr = this;

    passwd* pwd = getpwuid(getuid());
    if(!pwd)
    {
        throw std::runtime_error("pamHandler::pamHandler: getpwuid failed");
        return;
    }

    if(pam_start("login", pwd->pw_name, &pamConv_, &pam_) != PAM_SUCCESS)
    {
        throw std::runtime_error("pamHandler::pamHandler: pam_start failed");
        return;
    }

    std::string ttyname = ""; //todo
    if(pam_set_item(pam_, PAM_TTY, ttyname.c_str()) != PAM_SUCCESS)
    {
        throw std::runtime_error("pamHandler::pamHandler: pam_set_item for TTY failed");
        return;
    }

    if(pam_open_session(pam_, 0) != PAM_SUCCESS)
    {
        throw std::runtime_error("pamHandler::pamHandler: pam_open_session failed");
        return;
    }
}

pamHandler::~pamHandler()
{
    if(pam_)
    {
        pam_close_session(pam_, 0);
        pam_end(pam_, 0);
    }
}
