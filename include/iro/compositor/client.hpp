#pragma once

#include <iro/include.hpp>
#include <nytl/nonCopyable.hpp>
#include <nytl/callback.hpp>

#include <vector>
#include <memory>

namespace iro
{

///Client class that represent a wl_client and manages all its resources
///The Client class basically just holds a list of owned resources and the corresponding 
///wl_client pointer. When the client is disconnected, all resources are automatically 
///deleted (and destroyed by the wayland side).
class Client : public nytl::nonCopyable
{
public:
	struct listenerPOD;

public:
	///Returns the client object for a given wl_client.
	///If there is none, it returns nullptr.
	static Client* find(wl_client& res);

	///Returns the client object for a given wl_client.
	///If there is none, it returns nullptr and send a warning.
	static Client* findWarn(wl_client& res);

protected:
    wl_client* wlClient_;
	Compositor* compositor_;
	std::unique_ptr<listenerPOD> listener_;
	nytl::callback<void(Client&)> destructionCallback_;

    std::vector<std::unique_ptr<Resource>> resources_;

public:
    Client(Compositor& comp, wl_client& wlc);
    ~Client();

	///Destroys the wl_client. Will implicitly delete the client object and with that
	///all its resources.
	void destroy();

    ///Adds a resource to the clients resources.
    ///The client of the resource must match the given client object.
    void addResource(std::unique_ptr<Resource>&& res);

	template<typename R>
	R& addResource(std::unique_ptr<R>&& res)
	{
		R& ret = *res;
		addResource(std::unique_ptr<Resource>(res.release()));
		return ret;
	}

    ///Deletes res and removes it from the clients resource list.
    bool removeResource(const Resource& res);

	///Searches for its resource object of a given wl_resource. If there is none
	///it returns nullptr.
	Resource* findResource(const wl_resource& res) const;

	//todo: those 3 functions should not exist
    ///Returns the seat resource of the given client, nullptr if it has none.
    SeatRes* seatResource() const;

    ///Returns the pointer resource of the given client, nullptr if it has none.
    PointerRes* pointerResource() const;

    ///Returns the keyboard resource of the given client, nullptr if it has none.
    KeyboardRes* keyboardResource() const;

    ///Returns the corresponding wl_client object.
    wl_client& wlClient() const { return *wlClient_; }

	///Adds a callback which should be called when this client object is destructed.
	///Notice that the destruction of this client object does not autmatically mean
	///a destructino of the wl_client object. The callback function must have a signature
	///compatible to void(client&)
	template<typename F> nytl::connection onDestruction(F&& f)
		{ return destructionCallback_.add(f); }

    ///Returns the number of resources this client holds.
    std::size_t numberResources() const { return resources_.size(); }

	///Returns the compositor this client is connected to.
	Compositor& compositor() const { return *compositor_; }
};

}
