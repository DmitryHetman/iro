#pragma once

#include <iro.hpp>
#include <util/callback.hpp>

#include <functional>

class ttyHandler
{
protected:
    friend void ttySignalhandler(int);

    bool activate();

    void enteredTTY();
    void leftTTY();

    bool focus_ = 0;

    unsigned int number_;

    int fd_ = 0;

    callback<void()> beforeEnter_;
    callback<void()> beforeLeave_;

    callback<void()> afterEnter_;
    callback<void()> afterLeave_;

public:
    ttyHandler(sessionHandler& handler);
    ~ttyHandler();

    bool focus() const { return focus_; }
    unsigned int number() const { return number_; }
    int fd() const { return fd_; }

    connection& beforeEnter(std::function<void()> f){ return beforeEnter_.add(f); }
    connection& beforeLeave(std::function<void()> f){ return beforeLeave_.add(f); }

    connection& afterEnter(std::function<void()> f){ return afterEnter_.add(f); }
    connection& afterLeave(std::function<void()> f){ return afterLeave_.add(f); }
};

