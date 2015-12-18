#pragma once

#include <iro/include.hpp>
#include <nytl/nonCopyable.hpp>

#include <string>
#include <functional>
#include <vector>

//prototypes
typedef struct DBusMessage DBusMessage;
typedef struct DBusConnection DBusConnection;
typedef struct DBusError DBusError;

namespace iro
{


///Responsible for setting up and managing the dbus communication.
///Needed for taking session and then creating/releasing devices.
class DBusHandler : public nytl::nonCopyable
{
protected:
	struct Callbacks;
	struct MsgCallback
	{
		std::string interface;
		std::string member;
		std::function<void(DBusMessage*)> func;
	};

protected:
	Compositor* compositor_ = nullptr;
    DBusConnection* dbusConnection_ = nullptr;
    wl_event_source* dbusEventSource_ = nullptr;
	std::vector<MsgCallback> signalCallbacks_;

	void disconnected(DBusMessage* msg);
/*
	void sessionRemoved(DBusMessage* msg);
	void propertiesChanges(DBusMessage* msg);
	void devicePaused(DBusMessage* msg);
	void deviceResumed(DBusMessage* msg);
*/
	unsigned int filterMessage(DBusMessage* msg);

public:
	DBusHandler(Compositor& comp);
	~DBusHandler();

	Compositor& compositor() const { return *compositor_; }
	DBusConnection& dbusConnection() const { return *dbusConnection_; }

	//addMatch
	bool addSignalMatch(const std::string& sender, const std::string& interface, 
		const std::string& member, const std::string&, std::function<void(DBusMessage*)> f);

	//error
	static std::string checkError(DBusError& err);
	static bool checkError(DBusError& err, std::string& msg);
	static bool checkErrorThrow(DBusError& err);
	static bool checkErrorWarn(DBusError& err);
};

}
