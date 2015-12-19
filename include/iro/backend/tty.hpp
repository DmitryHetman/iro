#pragma once

#include <iro/include.hpp>
#include <nytl/callback.hpp>

#include <functional>

namespace iro
{

class TerminalHandler
{
protected:
    static int ttySignalhandler(int, void*);

    bool focus_ = 0;
    unsigned int number_;
    int tty_ = 0;

	nytl::callback<void()> beforeEnter_;
	nytl::callback<void()> beforeLeave_;

	nytl::callback<void()> afterEnter_;
	nytl::callback<void()> afterLeave_;

    void enteredTTY();
    void leftTTY();

public:
    TerminalHandler(Compositor& comp);
    ~TerminalHandler();

    bool focus() const { return focus_; }
    unsigned int number() const { return number_; }
    int ttyFd() const { return tty_; }

    bool activate(unsigned int vtNumber);
    bool waitActive(unsigned int vtNumber);

    template<typename F> nytl::connection beforeEnter(F&& f){ return beforeEnter_.add(f); }
    template<typename F> nytl::connection beforeLeave(F&& f){ return beforeLeave_.add(f); }

    template<typename F> nytl::connection afterEnter(F&& f){ return afterEnter_.add(f); }
    template<typename F> nytl::connection afterLeave(F&& f){ return afterLeave_.add(f); }
};

}
