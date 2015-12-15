#pragma once

#include <iro/include.hpp>
#include <nytl/callback.hpp>

#include <functional>

namespace iro
{

class TerminalHandler
{
protected:
    static void ttySignalhandler(int);

    bool focus_ = 0;
    unsigned int number_;
    Device* tty_ = nullptr;

	nytl::callback<void()> beforeEnter_;
	nytl::callback<void()> beforeLeave_;

	nytl::callback<void()> afterEnter_;
	nytl::callback<void()> afterLeave_;

    bool activate();
    void enteredTTY();
    void leftTTY();

public:
    TerminalHandler(DeviceHandler& dev);
    ~TerminalHandler();

    bool focus() const { return focus_; }
    unsigned int number() const { return number_; }
    Device* ttyDevice() const { return tty_; }

    template<typename F> nytl::connection beforeEnter(F&& f){ return beforeEnter_.add(f); }
    template<typename F> nytl::connection beforeLeave(F&& f){ return beforeLeave_.add(f); }

    template<typename F> nytl::connection afterEnter(F&& f){ return afterEnter_.add(f); }
    template<typename F> nytl::connection afterLeave(F&& f){ return afterLeave_.add(f); }
};

}
