#include <iro/backend/tty.hpp>
#include <iro/backend/devices.hpp>
#include <nytl/log.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <sys/ioctl.h>
#include <linux/vt.h>
#include <linux/kd.h>

#include <stdexcept>
#include <iostream>

namespace iro
{

//util
TerminalHandler* gInstance = nullptr;

void TerminalHandler::ttySignalhandler(int signal)
{
    if(signal == SIGUSR1) gInstance->enteredTTY();
    else if(signal == SIGUSR2) gInstance->leftTTY();
}

//TerminalHandler
TerminalHandler::TerminalHandler(DeviceHandler& dev)
{
	/*
    const char* number = getenv("XDG_VTNR");
    if(!number)
    {
        throw std::runtime_error("tty::tty: XDG_VTNR not set");
        return;
    }
	*/
	number_ = 3;


	//open tty
    std::string ttyString = "/dev/tty" + std::to_string(number_);
	tty_ = dev.createDevice(ttyString, O_RDWR | O_NOCTTY | O_CLOEXEC);
	if(!tty_ || !tty_->fd())
	{
        throw std::runtime_error("TerminalHandler::TerminalHandler: couldnt open " + ttyString);
        return;
	}

	int fd = tty_->fd();

    //save current
    vt_stat state;
    if(ioctl(fd, VT_GETSTATE, &state) == -1)
    {
        throw std::runtime_error("could not get current tty");
        return;
    }

    //set it up
    if(!activate())
    {
        throw std::runtime_error("Could not activate tty");
        return;
    }
    focus_ = 1;


    if(ioctl(fd, KDSETMODE, KD_GRAPHICS) == -1)
    {
        throw std::runtime_error("Could not set tty to graphics mode");
        return;
    }

    vt_mode mode;
    mode.mode = VT_PROCESS;
    mode.acqsig = SIGUSR1;
    mode.relsig = SIGUSR2;

    if(ioctl(fd, VT_SETMODE, &mode) == -1)
    {
        throw std::runtime_error("Could not set vt_mode");
        return;
    }

    //sig handler
    struct sigaction action;
    action.sa_handler = ttySignalhandler;

    sigaction(SIGUSR1, &action, nullptr);
    sigaction(SIGUSR2, &action, nullptr);

	//set global ~
	gInstance = this;

}

TerminalHandler::~TerminalHandler()
{
	nytl::sendLog("resetting terminal ", number_);
	*nytl::sendLog.stream << std::endl;

	if(!tty_) return;

    vt_mode mode;
    mode.mode = VT_AUTO;
    ioctl(tty_->fd(), VT_SETMODE, &mode);
    ioctl(tty_->fd(), KDSETMODE, KD_TEXT);

	tty_->release();
}

bool TerminalHandler::activate()
{
    if(focus_) return 1;

    if(ioctl(tty_->fd(), VT_ACTIVATE, number_) == -1) return 0;
    if(ioctl(tty_->fd(), VT_WAITACTIVE, number_) == -1) return 0;
    focus_ = 1;

    return 1;
}

void TerminalHandler::enteredTTY()
{
	nytl::sendLog("entered tty ", number_);
    beforeEnter_();

    ioctl(tty_->fd(), VT_RELDISP, VT_ACKACQ);
    focus_ = 1;

    afterEnter_();

    struct sigaction action;
    action.sa_handler = ttySignalhandler;

    sigaction(SIGUSR1, &action, nullptr);
    sigaction(SIGUSR2, &action, nullptr);
}

void TerminalHandler::leftTTY()
{
	nytl::sendLog("left tty ", number_);
    beforeLeave_();

    ioctl(tty_->fd(), VT_RELDISP, 1); //allowed
    focus_ = 0;

    afterLeave_();

    struct sigaction action;
    action.sa_handler = ttySignalhandler;

    sigaction(SIGUSR1, &action, nullptr);
    sigaction(SIGUSR2, &action, nullptr);
}

}
