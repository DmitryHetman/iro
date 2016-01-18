#pragma once

#include <iro/include.hpp>
#include <nytl/nonCopyable.hpp>
#include <nytl/callback.hpp>
#include <nytl/watchable.hpp>

namespace iro
{


namespace resourceType
{
    constexpr unsigned int unknown = 0;

    constexpr unsigned int surface = 1;
    constexpr unsigned int shellSurface = 2;
    constexpr unsigned int subsurface = 3;
    constexpr unsigned int region = 4;
    constexpr unsigned int pointer = 5;
    constexpr unsigned int keyboard = 6;
    constexpr unsigned int touch = 7;
    constexpr unsigned int buffer = 8;
    constexpr unsigned int dataDevice = 9;
    constexpr unsigned int dataSource = 10;
    constexpr unsigned int dataOffer = 11;
    constexpr unsigned int callback = 12;

    //globals
    constexpr unsigned int seat = 13;
    constexpr unsigned int compositor = 14;
    constexpr unsigned int subcompositor = 15;
    constexpr unsigned int output = 16;
    constexpr unsigned int shell = 17;
    constexpr unsigned int dataDeviceManager = 18;
};


///The resource class represents a wayland wl_resource.
/** Resource is the base class of all possible resource representations on the server.
 * It holds the wl_resource pointer and its lifetime can be observed, since it has a 
 * destructionCallback, which makes resource references possible.
 * An resource or resource-derived typed object shall never be deleted, since the client 
 * object has the ownership.
 * If a resource should be destroyed, call the destroy() function, which will destroy the 
 * wl_resource and trigger a destruction of the resource object.
 * Since the resource class is also notified when the wl_resource object was destroyed (on server
 * as well as on client side), it is also sufficient to just call wl_resource_destroy for a
 * given wl_resource.
 *
 * The wl_resource pointer of a resource object shall alwayas point to a valid wl_resource 
 * (after creation, creating them without a valid wl_resource will trigger many problems), since 
 * they will be automatically destroyed if their corresponding wl_resource is destructed.
**/
class Resource : public nytl::nonCopyable, public nytl::watchable
{
public:
	///POD structure used to get the associated resource object from a wl_resource pointer
	struct listenerPOD;

protected:
	///Sends an invalid object error to the given resource and disconnects its client.
	static void invalidObjectDisconnect(wl_resource& res, const std::string& info);	

public:
	///Returns the resource object for a given wl_resource.
	///If there is none, it returns nullptr.
	static Resource* find(wl_resource& res);

	///Validates that there is a resource object of type T associated with the 
	///given wl_resource and returns this object.
	///If there is no resource object at all associated with the
	///wl_resource or it is not of type T, validate returns a nullptr.
	template<typename T> 
	static T* validate(wl_resource& res)
	{
		auto rs = find(res);
		return rs ? dynamic_cast<T*>(rs) : nullptr;
	};

	///Validates that there is a resource object of type T associated with the 
	///given wl_resource and returns is. If there is no such object associated, it returns 
	///a nullptr, sends a WL_DISPLAY_INVALID_OBJECT to the resource and destroys the client.
	template<typename T>
	static T* validateDisconnect(wl_resource& res, const std::string& info = {})
	{
		auto rs = validate<T>(res);
		if(!rs) invalidObjectDisconnect(res, info);

		return rs;
	};

	///Validates a resource pointer. Fails when the pointer is a nullptr, else it calls
	///the reference version.
	template<typename T>
	static T* validateDisconnect(wl_resource* res, const std::string& info = {})
	{
		if(!res) return nullptr; //warning, in source file impl

		return validateDisconnect<T>(*res, info);
	}

protected:
    wl_resource* wlResource_ = nullptr;
	nytl::callback<void(Resource&)> destructionCallback_;
	std::unique_ptr<listenerPOD> listener_;

    Resource() = default;
    Resource(wl_client& client, unsigned int id, const wl_interface* interface, 
			const void* implementation, unsigned int version = 1);

    void create(wl_client& client, unsigned int id, const wl_interface* interface, 
			const void* implementation, unsigned int version = 1);

public:
    Resource(wl_resource& res);
    virtual ~Resource();

    ///Destroys the wl_resource object which will trigger its own destruction. 
    virtual void destroy();

    ///Returns the wayland id of the resource
    unsigned int id() const;

    ///Returns the wayland version of the wayland resource implementation
    unsigned int version() const;

    ///Returns the corresponding wl_resource
    wl_resource& wlResource() const { return *wlResource_; }

    ///Returns the wl_client that holds this resource
    wl_client& wlClient() const;

    ///Returns the registered client object for the clients that holds this resource
    Client& client() const;

	//Returns the compositor this resource belongs to
	Compositor& compositor() const;

	///Returns the id of the resource type.
	virtual unsigned int type() const { return resourceType::unknown; }
};

///\related resource
///checks two resources for equality (if both of them point to the same wl_resource object)
bool operator==(const Resource& r1, const Resource& r2);

}
