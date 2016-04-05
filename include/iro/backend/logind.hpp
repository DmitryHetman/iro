#pragma once

#include <iro/include.hpp>
#include <nytl/nonCopyable.hpp>
#include <nytl/callback.hpp>

#include <string>

//prototypes
typedef struct DBusMessage DBusMessage;
typedef struct DBusMessageIter DBusMessageIter;
typedef struct DBusPendingCall DBusPendingCall;

namespace iro
{

///Manages the session with logind.
class LogindHandler : public nytl::NonCopyable
{
protected:
	DBusHandler* dbus_ = nullptr;
	std::string session_;
	std::string sessionPath_;

	DBusPendingCall* dbusPendingActive_ = nullptr;
	nytl::Callback<void(bool)> activeCallback_;

	void dbusSessionRemoved(DBusMessage* msg);

	void dbusPropertiesChanged(DBusMessage* msg);
	void dbusGetActive();
	void dbusParseActive(DBusMessage* m, DBusMessageIter* it);
	static void dbusGetActiveCallback(DBusPendingCall* call, void* data);

public:
	LogindHandler(DBusHandler& dbus);
	~LogindHandler();

	DBusHandler& dbusHandler() const { return *dbus_; };
	Compositor& compositor() const;

	template<typename F> nytl::Connection onActive(F&& f){ return activeCallback_.add(f); }

	const std::string& sessionPath() const { return sessionPath_; }
	const std::string& session() const { return session_; }
};

}
