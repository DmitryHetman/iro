#pragma once

#include <iro/include.hpp>
#include <nytl/callback.hpp>
#include <nytl/nonCopyable.hpp>

#include <functional>

namespace iro
{

///Responsible for managing setup and switching of the virtual terminal. Therefore only needed
///if iro is started outside an existent window manager (drm/kms backend).
class TerminalHandler : public nytl::NonCopyable
{
protected:
    static int ttySignalhandler(int, void*);

protected:
    bool focus_ = 0;
    unsigned int number_;
    int tty_ = 0;

	nytl::Callback<void()> beforeEnter_;
	nytl::Callback<void()> beforeLeave_;

	nytl::Callback<void()> afterEnter_;
	nytl::Callback<void()> afterLeave_;

protected:
    void enteredTTY();
    void leftTTY();

public:
    TerminalHandler(Compositor& comp);
    ~TerminalHandler();

	///Returns whether the virtual terminal iro runs on has focus.
    bool focus() const { return focus_; }

	///Returns the virtual terminal number iro runs on.
    unsigned int number() const { return number_; }

	///Returns the used fd to the virtual terminal device.
    int ttyFd() const { return tty_; }

	///Activated the given virtual terminal, and waits for activation.
	///Returns 0 on failure.
    bool activate(unsigned int vtNumber);

    template<typename F> nytl::Connection beforeEnter(F&& f){ return beforeEnter_.add(f); }
    template<typename F> nytl::Connection beforeLeave(F&& f){ return beforeLeave_.add(f); }

    template<typename F> nytl::Connection afterEnter(F&& f){ return afterEnter_.add(f); }
    template<typename F> nytl::Connection afterLeave(F&& f){ return afterLeave_.add(f); }
};

}
