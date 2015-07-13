#pragma once

#include <iro.hpp>
#include <vector>

class client
{
protected:
    friend compositor;
    client(wl_client* wlc);

    wl_client* wlClient_ = nullptr;

    std::vector<resource*> resources_;
    seatRes* seat_ = nullptr;

public:
    ~client();

    void addResource(resource* res);
    void removeResource(resource* res);

    seatRes* getSeatRes() const { return seat_; }
    wl_client* getWlClient() const { return wlClient_; }
};
