#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>

#include <nyutil/nonCopyable.hpp>

#include <unordered_map>
#include <vector>

///////////////////////////////////////////
class compositor : public nonCopyable
{
protected:
    wl_display* wlDisplay_ = nullptr;
    wl_global* global_ = nullptr;

    seat* seat_ = nullptr;
    subcompositor* subcompositor_ = nullptr;

    std::unordered_map<wl_client*, client*> clients_;
    std::unordered_map<unsigned int, event*> sentEvents_; //memory leak! delete old events every now and then

    static compositor* object;

public:
    compositor();
    ~compositor();

    void run();

    shell* getShell() const { return shell_; }
    seat* getSeat() const { return seat_; }
    subcompositor* getSubcompositor() const { return subcompositor_; }

    unsigned int getNumberClients() const { return clients_.size(); }
    client& getClient(wl_client& wlc); //getOrCreate -> always returns a valid client, reference
    bool registeredClient(wl_client& wlc); //checks if client is registered
    void unregisterClient(client& c);

    wl_display* getWlDisplay() const { return wlDisplay_; }
    wl_event_loop* getWlEventLoop() const;

    event* getEvent(unsigned int serial) const; //gets sent event for a given serial
    unsigned int registerEvent(event& ev); //increases wl_display serial, returns it and registers the event for this serial

    static compositor* getObject(){ return object; }
};

//////////////////////////
class compositorRes : public resource
{
public:
    compositorRes(wl_client& client, unsigned int id, unsigned int version);

    //res
    virtual resourceType getType() const override { return resourceType::compositor; }
};
