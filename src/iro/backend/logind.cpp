#include <iro/backend/logind.hpp>
#include <iro/backend/dbus.hpp>
#include <iro/compositor/compositor.hpp>

#include <nytl/misc.hpp>
#include <nytl/log.hpp>

#include <dbus/dbus.h>
#include <systemd/sd-login.h>

#include <sys/types.h>
#include <unistd.h>

#include <string>

namespace iro
{

LogindHandler::LogindHandler(DBusHandler& dbus) : dbus_(&dbus)
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

	nytl::sendLog("logind session: ", session_, " path: ", sessionPath_);

	//todo: use them in some way. They are guessed elsewhere -> connect
    char* seat = nullptr;
    if(sd_session_get_seat(session_.c_str(), &seat) < 0 || !seat)
    {
        throw std::runtime_error("cant get seat from systemd");
        return;
    }
	nytl::sendLog("logind seat: ", seat);
    free(seat);

	unsigned int vt;
    if(sd_session_get_vt(session_.c_str(), &vt) < 0)
    {
        throw std::runtime_error("cant get vt number from systemd");
        return;
    };
	nytl::sendLog("logind vt: ", vt);

	//
	const std::string path("org.freedesktop.login1");
    if(!dbus.addSignalMatch(path, "org.freedesktop.login1.Manager", "SessionRemoved",
		"/org/freedesktop/login1", nytl::memberCallback(&LogindHandler::dbusSessionRemoved, this)))
	{
        throw std::runtime_error("Logind: add Signal match for sessionRemoved failed");
        return;
	}

    if(!dbus.addSignalMatch(path, "org.freedesktop.DBus.Properties", "PropertiesChanged", 
		sessionPath(), nytl::memberCallback(&LogindHandler::dbusPropertiesChanged, this)))
	{
        throw std::runtime_error("Logind: add Signal match for PropertiesChanges failed");
        return;
	}

	//take session control
	//todo: query errors and print them
    DBusMessage* msg;
	DBusError err;
	dbus_error_init(&err);
	std::string errStr;

    if(!(msg = dbus_message_new_method_call("org.freedesktop.login1", sessionPath().c_str(), 
		"org.freedesktop.login1.Session", "TakeControl")))
    {
        throw std::runtime_error("Logind: dbus call TakeControl failed");
        return;
    }

    dbus_bool_t arr[] = {false};
    if(!dbus_message_append_args(msg, DBUS_TYPE_BOOLEAN, arr, DBUS_TYPE_INVALID))
    {
        throw std::runtime_error("Logind: dbus_append_args for the take control msg failed");
        return;
    }

    DBusMessage* reply = nullptr;
    if(!(reply = dbus_connection_send_with_reply_and_block(&dbusHandler().dbusConnection(), 
			msg, -1, &err)) || dbus.checkError(err, errStr) || !reply)
    {
        throw std::runtime_error("Logind:sending the takeControl msg failed. " + errStr);
        return;
    }

    dbus_message_unref(reply);
    dbus_message_unref(msg);

	nytl::sendLog("Took Session Control, finished Logind setup");
}

LogindHandler::~LogindHandler()
{
    DBusMessage* m;
    if (!(m = dbus_message_new_method_call("org.freedesktop.login1", sessionPath().c_str(), 
			"org.freedesktop.login1.Session", "ReleaseControl")))
    {
		nytl::sendWarning("~LogindHandler: dbus new method call ReleaseControl failed.");
    }
	else
	{
		dbus_connection_send(&dbusHandler().dbusConnection(), m, nullptr);
		dbus_message_unref(m);
		nytl::sendLog("~LogindHandler: succesfully released control of session");
	}
}

void LogindHandler::dbusSessionRemoved(DBusMessage* msg)
{
    const char* name;
    const char* obj;
    if(!dbus_message_get_args(msg, nullptr, DBUS_TYPE_STRING, &name, DBUS_TYPE_OBJECT_PATH, 
			&obj, DBUS_TYPE_INVALID) || name != sessionPath())
	{
		nytl::sendLog("dbus session was removed but invalid session path -> ignoring it");
		return;
	}

	nytl::sendWarning("dbus session was removed. exiting");
    compositor().exit();
}

Compositor& LogindHandler::compositor() const
{
	return dbusHandler().compositor();
}

void LogindHandler::dbusPropertiesChanged(DBusMessage* m)
{
	nytl::sendLog("props changes");

    DBusMessageIter iter;
    if (!dbus_message_iter_init(m, &iter) || dbus_message_iter_get_arg_type(&iter) != 
			DBUS_TYPE_STRING) 
		goto error0;

    const char *interface;
    dbus_message_iter_get_basic(&iter, &interface);

    if (!dbus_message_iter_next(&iter) || 
			dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
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
            dbusParseActive(m, &entry);
            return;
        }

        dbus_message_iter_next(&sub);
    }

    if (!dbus_message_iter_next(&iter) || 
			dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
        goto error0;

    dbus_message_iter_recurse(&iter, &sub);

    while (dbus_message_iter_get_arg_type(&sub) == DBUS_TYPE_STRING)
    {
        const char *name;
        dbus_message_iter_get_basic(&sub, &name);

        if(std::string(name) == "Active")
        {
            dbusGetActive();
            return;
        }

        dbus_message_iter_next(&sub);
    }

    return;

error0:
	nytl::sendWarning("LoigndHandler: cannot parse PropertiesChanged dbus signal");
}

void LogindHandler::dbusGetActiveCallback(DBusPendingCall* pending, void* data)
{
	if(!data)
	{
		nytl::sendWarning("nytl error dummy");
		return;
	}
	LogindHandler* dh = static_cast<LogindHandler*>(data);

	dbus_pending_call_unref(dh->dbusPendingActive_);
	dh->dbusPendingActive_ = nullptr;

	DBusMessage *m;
	if (!(m = dbus_pending_call_steal_reply(pending)))
	{
		nytl::sendWarning("nytl error dummy");
		return;
	}

	DBusMessageIter iter;
	if(dbus_message_get_type(m) == DBUS_MESSAGE_TYPE_METHOD_RETURN && 
		dbus_message_iter_init(m, &iter))
      dh->dbusParseActive(m, &iter);

   dbus_message_unref(m);
}

void LogindHandler::dbusGetActive()
{
	DBusMessage *m;
   	if(!(m = dbus_message_new_method_call("org.freedesktop.login1", sessionPath().c_str(), 
			"org.freedesktop.DBus.Properties", "Get")))
	{
		nytl::sendWarning("nytl error dummy");
		return;
	}

   	const char *iface = "org.freedesktop.login1.Session";
  	const char *name = "Active";
   	if (!dbus_message_append_args(m, DBUS_TYPE_STRING, &iface, DBUS_TYPE_STRING, 
		&name, DBUS_TYPE_INVALID))
	{
		nytl::sendWarning("nytl error dummy");
		dbus_message_unref(m);
		return;
	}

	DBusPendingCall *pending;
	if (!dbus_connection_send_with_reply(&dbusHandler().dbusConnection(), m, &pending, -1))
	{
		nytl::sendWarning("nytl error dummy");
		dbus_message_unref(m);
		return;
	}

	if (!dbus_pending_call_set_notify(pending, &LogindHandler::dbusGetActiveCallback, 
				this, nullptr))
	{
		nytl::sendWarning("nytl error dummy");
		dbus_pending_call_cancel(pending);
		dbus_pending_call_unref(pending);
		dbus_message_unref(m);
		return;
	}

	if (dbusPendingActive_)
	{
		dbus_pending_call_cancel(dbusPendingActive_);
		dbus_pending_call_unref(dbusPendingActive_);
	}

	dbusPendingActive_ = pending;
}

void LogindHandler::dbusParseActive(DBusMessage* msg, DBusMessageIter* it)
{
	if(dbus_message_iter_get_arg_type(it) != DBUS_TYPE_VARIANT)
	{
		nytl::sendWarning("nytl error dummy");
		return;
	}

   DBusMessageIter sub;
   dbus_message_iter_recurse(it, &sub);

   if(dbus_message_iter_get_arg_type(&sub) != DBUS_TYPE_BOOLEAN)
   {
		nytl::sendWarning("nytl error dummy");
		return;
   }

   dbus_bool_t b;
   dbus_message_iter_get_basic(&sub, &b);
   activeCallback_(static_cast<bool>(b));
}

}
