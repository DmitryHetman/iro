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
		"/org/freedesktop/login1", nytl::memberCallback(&LogindHandler::sessionRemoved, this)))
	{
        throw std::runtime_error("Logind: add Signal match for sessionRemoved failed");
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

void LogindHandler::sessionRemoved(DBusMessage* msg)
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

}
