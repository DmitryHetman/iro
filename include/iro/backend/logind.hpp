#pragma once

#include <iro/include.hpp>
#include <nytl/nonCopyable.hpp>

#include <string>

//prototypes
typedef struct DBusMessage DBusMessage;

namespace iro
{

///Manages the session taking with logind.
class LogindHandler : public nytl::nonCopyable
{
protected:
	DBusHandler* dbus_ = nullptr;
	std::string session_;
	std::string sessionPath_;

	void sessionRemoved(DBusMessage* msg);
	void propertiesChanged(DBusMessage* msg);

public:
	LogindHandler(DBusHandler& dbus);
	~LogindHandler();

	DBusHandler& dbusHandler() const { return *dbus_; };
	Compositor& compositor() const;

	const std::string& sessionPath() const { return sessionPath_; }
	const std::string& session() const { return session_; }
};

}
