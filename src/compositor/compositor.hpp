#pragma once

#include <iro.hpp>
#include <resources/resource.hpp>

#include <util/nonCopyable.hpp>

#include <unordered_map>
#include <vector>

///////////////////////////////////////////
class compositor : public nonCopyable
{
protected:
    wl_display* wlDisplay_ = nullptr;

    shell* shell_ = nullptr;
    seat* seat_ = nullptr;
    subcompositor* subcompositor_ = nullptr;

    std::unordered_map<wl_client*, client*> clients_;
    std::unordered_map<unsigned int, event*> sentEvents_; //memory leak! delete old events every now and then

    static compositor* object;

public:
    compositor();
    ~compositor();

    int run();

    shell* getShell() const { return shell_; }
    seat* getSeat() const { return seat_; }
    subcompositor* getSubcompositor() const { return subcompositor_; }

    wl_display* getWlDisplay() const { return wlDisplay_; }
    wl_event_loop* iroWlEventLoop() const;

    unsigned int getNumberClients() const { return clients_.size(); }
    client& getClient(wl_client& wlc); //getOrCreate -> always returns a valid client, reference
    void unregisterClient(client& c);

    event* getEvent(unsigned int serial) const; //gets sent event for a given serial
    void registerEvent(event& ev); //registers event for the CURRENT wl_display serial -> first send event with iroNextSerial() THEN register it

    static compositor* getObject(){ return object; }
};

//////////////////////////
class compositorRes : public resource
{
public:
    compositorRes(wl_client& client, unsigned int id, unsigned int version);

    resourceType getType() const { return resourceType::compositor; }
};
