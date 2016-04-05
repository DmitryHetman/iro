#pragma once

#include <iro/include.hpp>
#include <iro/util/global.hpp>
#include <nytl/time.hpp>

#include <memory>
#include <vector>

namespace iro
{

///The compositor class is responsible for managing wayland clients and surfaces.
///The compositor class is the main Wayland class.
///It holds the wl_display pointer and keeps track of all (owned) clients.
///On destruction, all clients are automatically disconnected and the wayland 
///compositor is destroyed.
class Compositor : public Global
{
protected:
    wl_display* wlDisplay_ = nullptr;
    std::vector<std::unique_ptr<Client>> clients_;
	nytl::Timer timer_;
    std::vector<std::unique_ptr<Event>> eventList_; 

	Backend* backend_ {nullptr};
	ShellModule* shell_ {nullptr};

	std::unique_ptr<Subcompositor> subcompositor_;

public:
    Compositor();
    ~Compositor();

    ///Runs the wayland display until it stops or exit() is called.
    void run();

    ///Runs the wayland display for the given amount of time.
    void run(const nytl::TimeDuration& time);

	///terminates the display event loop
	void exit();

    ///returns the current number of connected clients
    unsigned int numberClients() const { return clients_.size(); }

    ///checks if a wl_client is registered
    Client* clientRegistered(wl_client& wlc) const;

    ///returns the client object for a given wl_client pointer. if the wl_client is not registered
    ///at the moment, a new client object will be created. more like a "getOrCreateClient" function.
    Client& client(wl_client& wlc);

    ///Only unregisters and therfore destructs a client object. The wl_client object is unaffected, 
	///this is called in the clientDestroy listener.
    void unregisterClient(Client& c);

    ///returns the wl_display pointer
    wl_display& wlDisplay() const { return *wlDisplay_; }

    ///returns the wl_event_loop pointer of the used wl_display
    wl_event_loop& wlEventLoop() const;

	///returns the used backend, or nullptr if none is set
	Backend* backend() const { return backend_; }

	///Sets the used backend.
	///Note that the backend can only be set before the compositor starts to run
	///the mainLoop.
	void backend(Backend& bck){ backend_ = &bck; }

	///Registers the associated shellModule.
	void shell(ShellModule& shell) { shell_ = &shell;}

	///Returns the associated shell module.
	ShellModule* shell() const { return shell_; }

	///Registers an event and returns a reference to the registered Event object.
	///If the serial parameter is set, it sets and updates the current serial. 
	Event& event(std::unique_ptr<Event>&& ptr, bool serial = 0);

	///Returns the registered event for a given serial, nullptr if none is found.
	Event* event(unsigned int serial) const;

	///Returns the current wayland time stamp. Returns the time since compositor start in
	///milliseconds. Used for some wayland event functions.
	unsigned int time() const;

	///Returns the subcompositor object associated with this compositor.
	Subcompositor& subcompositor() const { return *subcompositor_; }
};

}
