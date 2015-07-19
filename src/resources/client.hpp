#pragma once

#include <iro.hpp>
#include <vector>

class client
{
protected:
    friend compositor; //only compositor can create new clients. done implicitly by getClient()
    client(wl_client& wlc);

    wl_client& wlClient_;

    std::vector<resource*> resources_;

    mutable seatRes* seat_ = nullptr;

public:
    ~client();

    void addResource(resource& res);
    bool removeResource(resource& res);

    seatRes* getSeatRes() const;
    pointerRes* getPointerRes() const;
    keyboardRes* getKeyboardRes() const;

    wl_client& getWlClient() const { return wlClient_; }
};
