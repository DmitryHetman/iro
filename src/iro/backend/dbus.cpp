#include <iro/backend/dbus.hpp>
#include <iro/compositor/compositor.hpp>

#include <nytl/log.hpp>
#include <nytl/misc.hpp>

#include <wayland-server-core.h>
#include <dbus/dbus.h>

#include <sys/eventfd.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdexcept>

namespace iro
{

//dbus callbacks
struct DBusHandler::Callbacks
{
	static unsigned int addWatch(DBusWatch* watch, void* data);
	static void removeWatch(DBusWatch* watch, void* data);
	static void toggleWatch(DBusWatch* watch, void* data);

	static unsigned int addTimeout(DBusTimeout* timeout, void* data);
	static void removeTimeout(DBusTimeout* timeout, void* data);
	static void toggleTimeout(DBusTimeout* timeout, void* data);

	static int dispatchDBus(int, unsigned int, void*);
	static int dispatchWatch(int, unsigned int, void*);
	static int dispatchTimeout(void*);

	static DBusHandlerResult dbusFilter(DBusConnection* conn, DBusMessage* msg, void* data);
};

//utility
int adjustTimeout(DBusTimeout* timeout, wl_event_source* source)
{
   unsigned long iv = 0;
   if (dbus_timeout_get_enabled(timeout))
      iv = dbus_timeout_get_interval(timeout);

   return wl_event_source_timer_update(source, iv);
}

//DbusHandler
DBusHandler::DBusHandler(Compositor& comp)
	: compositor_(&comp)
{
    DBusError err;
    dbus_error_init(&err);

    dbus_connection_set_change_sigpipe(false);

	//todo: error checking everywhere
    if(!(dbusConnection_ = dbus_bus_get_private(DBUS_BUS_SYSTEM, &err)))
    {
		std::string errStr;
		checkError(err, errStr);
			
        throw std::runtime_error("DBus::DBus: cant connect to dbus. " + errStr);
        return;
    }

    dbus_connection_set_exit_on_disconnect(dbusConnection_, false);

    //use dummy event fd
    int fd;
    if((fd = eventfd(0, EFD_CLOEXEC)) < 0)
    {
        throw std::runtime_error("DBus::DBus: cant create eventfd");
        return;
    }

	//event source
    dbusEventSource_ = wl_event_loop_add_fd(&comp.wlEventLoop(), fd, 0, Callbacks::dispatchDBus, 
			this);
    close(fd);
    wl_event_source_check(dbusEventSource_);

	//watch
    if(!dbus_connection_set_watch_functions(dbusConnection_, Callbacks::addWatch, 
				Callbacks::removeWatch, Callbacks::toggleWatch, this, nullptr))
    {
        throw std::runtime_error("dbus_connection_set_watch_functions failed");
        return;
    }

	//timeout
    if(!dbus_connection_set_timeout_functions(dbusConnection_, Callbacks::addTimeout, 
				Callbacks::removeTimeout, Callbacks::toggleTimeout, this, nullptr))
    {
        throw std::runtime_error("dbus_connection_set_timeout_functions failed");
        return;
    }

	//filter
    if(!dbus_connection_add_filter(dbusConnection_, Callbacks::dbusFilter, this, nullptr))
	{
		throw std::runtime_error("dbus add filter failed");
		return;
	}

	//disconnected callback
	signalCallbacks_.emplace_back(MsgCallback{DBUS_INTERFACE_LOCAL, "Diconnected", 
		nytl::memberCallback(&DBusHandler::disconnected, this)});

	nytl::sendLog("dbus handler succesfully set up");
}

DBusHandler::~DBusHandler()
{
	if(dbusConnection_)
	{
		dbus_connection_close(dbusConnection_);
		dbus_connection_unref(dbusConnection_);
	}
}

unsigned int DBusHandler::filterMessage(DBusMessage* msg)
{
	nytl::sendLog("dbus filter: ", dbus_message_get_member(msg));
	for(auto& cb : signalCallbacks_)
	{
		if(dbus_message_is_signal(msg, cb.interface.c_str(), cb.member.c_str()))
		{
			cb.func(msg);
			break;
		}
	}

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void DBusHandler::disconnected(DBusMessage* msg)
{
	nytl::sendLog("DBusHandler: received disconnected signal");
	//exit?
	//compositor().exit();
}

bool DBusHandler::addSignalMatch(const std::string& sender, const std::string& interface, 
	const std::string& member, const std::string& path, std::function<void(DBusMessage*)> f)
{
	std::string match = "type='signal'";
	match.append(",sender='" + sender + "'");
	match.append(",interface='" + interface + "'");
	match.append(",member='" + member + "'");
	match.append(",path='" + path + "'");

	DBusError err;
	dbus_error_init(&err);

	dbus_bus_add_match(dbusConnection_, match.c_str(), &err);
	bool success = !checkErrorWarn(err);

	if(!success) return 0;

	signalCallbacks_.emplace_back(MsgCallback{interface, member, f});
	return 1;
}

std::string DBusHandler::checkError(DBusError& err)
{
    if(dbus_error_is_set(&err))
    {
        std::string msg = "DBus Error " + std::string(err.name) + ": " + std::string(err.message);
        dbus_error_free(&err);
        return msg;
    }

    return std::string();
}

bool DBusHandler::checkError(DBusError& err, std::string& str)
{
    if(dbus_error_is_set(&err))
    {
        str = "DBus Error " + std::string(err.name) + ": " + std::string(err.message);
        dbus_error_free(&err);
        return 1;
    }

    return 0;
}

bool DBusHandler::checkErrorThrow(DBusError& err)
{
    std::string ret;
    if(checkError(err, ret))
    {
        throw std::runtime_error("dbus error: " + ret);
        return 1;
    }

    return 0;
}

bool DBusHandler::checkErrorWarn(DBusError& err)
{
    std::string ret;
    if(checkError(err, ret))
    {
		nytl::sendWarning("dbus error: ", ret);
        return 1;
    }

    return 0;
}

//Callbacks implementation
unsigned int DBusHandler::Callbacks::addWatch(DBusWatch* watch, void* data)
{
	if(!data) return 0;
	auto& loop = static_cast<DBusHandler*>(data)->compositor().wlEventLoop();

    unsigned int mask = 0;
    if (dbus_watch_get_enabled(watch))
    {
        unsigned int flags = dbus_watch_get_flags(watch);

        if (flags & DBUS_WATCH_READABLE) mask |= WL_EVENT_READABLE;
        if (flags & DBUS_WATCH_WRITABLE) mask |= WL_EVENT_WRITABLE;
    }

    int fd = dbus_watch_get_unix_fd(watch);
    wl_event_source* source = wl_event_loop_add_fd(&loop, fd, mask, dispatchWatch, watch);

    if (!source)
	{
		nytl::sendWarning("failed to add dbus watch wl_event_loop_fd");
		return 0;
	}

    dbus_watch_set_data(watch, source, nullptr);
    return 1;
}
void DBusHandler::Callbacks::removeWatch(DBusWatch* watch, void* data)
{
   wl_event_source* s = nullptr;
   if (!(s = (wl_event_source*) dbus_watch_get_data(watch)))
   {
		nytl::sendWarning("dbusRemoveWatch: dbus watch has no data");
        return;
   }

   wl_event_source_remove(s);
}
void DBusHandler::Callbacks::toggleWatch(DBusWatch* watch, void* data)
{
    struct wl_event_source *s;
    if(!(s = (wl_event_source*) dbus_watch_get_data(watch)))
    {
		nytl::sendWarning("dbusToggleWatch: dbus watch has no data");
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

unsigned int DBusHandler::Callbacks::addTimeout(DBusTimeout* timeout, void* data)
{
	if(!data) return 0;
	auto& loop = static_cast<DBusHandler*>(data)->compositor().wlEventLoop();

    wl_event_source* source;
    if(!(source = wl_event_loop_add_timer(&loop, dispatchTimeout, timeout)))
    {
		nytl::sendWarning("dbusAddTimeout: failed to add wl_event_loop_timer");
        return 0;
    }

    if(adjustTimeout(timeout, source) < 0)
    {
		nytl::sendWarning("dbusAddTimeout: failed to adjust timeout");
        wl_event_source_remove(source);
        return 0;
    }

    dbus_timeout_set_data(timeout, source, nullptr);
    return true;
}
void DBusHandler::Callbacks::removeTimeout(DBusTimeout* timeout, void* data)
{
	wl_event_source* s = (wl_event_source*) dbus_timeout_get_data(timeout);
    if(!s)
    {
		nytl::sendWarning("dbusRemoveTimeout: dbus timeout has no data");
        return;
    }

    wl_event_source_remove(s);
}
void DBusHandler::Callbacks::toggleTimeout(DBusTimeout* timeout, void* data)
{
    wl_event_source* s = (wl_event_source*) dbus_timeout_get_data(timeout);
    if(!s)
    {
		nytl::sendWarning("dbusToggleTimeout: dbus timeout has no data");
        return;
    }

    adjustTimeout(timeout, s);
}
int DBusHandler::Callbacks::dispatchDBus(int, unsigned int, void* data)
{
	if(!data) return 0;

	auto handler = static_cast<DBusHandler*>(data);
	while(1)
	{
		switch(dbus_connection_dispatch(&handler->dbusConnection()))
        {
            case DBUS_DISPATCH_DATA_REMAINS: break;
            case DBUS_DISPATCH_COMPLETE: return 0;
			case DBUS_DISPATCH_NEED_MEMORY: nytl::sendWarning("DBUS_DISPATCH_NEED_MEMORY"); 
											return 0;
			default: nytl::sendWarning("DBUS_DISPATCH_ERROR"); return 0;
        }
   }

   return 0;
}

int DBusHandler::Callbacks::dispatchWatch(int, unsigned int mask, void* data)
{
	if(!data) return 0;
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

int DBusHandler::Callbacks::dispatchTimeout(void* data)
{
   DBusTimeout* timeout = (DBusTimeout*) data;

   if (dbus_timeout_get_enabled(timeout))
      dbus_timeout_handle(timeout);

   return 0;
}

DBusHandlerResult DBusHandler::Callbacks::dbusFilter(DBusConnection*, DBusMessage* msg, void* data)
{
	if(!data) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	auto handler = static_cast<DBusHandler*>(data);

	return DBusHandlerResult(handler->filterMessage(msg));
}

}
