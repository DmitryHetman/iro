#pragma once

#include <iro.hpp>
#include <vector>

class client
{
protected:
    friend compositor;
    client(wl_client& wlc);

    wl_client& wlClient_;

    std::vector<resource*> resources_;
    seatRes* seat_ = nullptr;

public:
    ~client();

    void addResource(resource& res);
    bool removeResource(resource& res);

    seatRes* iroSeatRes() const { return seat_; }
    wl_client& getWlClient() const { return wlClient_; }
};
