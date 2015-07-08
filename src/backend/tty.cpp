#include <backend/tty.hpp>

//C
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <sys/ioctl.h>
#include <linux/vt.h>
#include <linux/kd.h>

//CPP
#include <stdexcept>
#include <iostream>

void ttySignalhandler(int signal)
{
    ttyHandler* handler = getTTYHandler();
    if(!handler) return;

    if(signal == SIGUSR1) handler->enteredTTY();
    else if(signal == SIGUSR2) handler->leftTTY();
}

ttyHandler::ttyHandler() : focus_(0)
{
    //tty0
    int tty0FD = open("/dev/tty0", O_RDWR | O_CLOEXEC);
    if(tty0FD < 0)
    {
        throw std::runtime_error("could not open tty0");
        return;
    }

    if(ioctl(tty0FD, VT_OPENQRY, &number_) != 0)
    {
        throw std::runtime_error("no free tty found");
        return;
    }

    close(tty0FD);

    ////
    //open own tty
    std::string ttyString;
    ttyString += "/dev/tty";
    ttyString += std::to_string(number_);

    if((fd_ = open(ttyString.c_str(), O_RDWR | O_NOCTTY | O_CLOEXEC)) < 0)
    {
        throw std::runtime_error("could not open own tty");
        return;
    }

    //set it up
    if(!activate())
    {
        throw std::runtime_error("Could not activate tty");
        return;
    }

    if(ioctl(fd_, KDSETMODE, KD_GRAPHICS) == -1)
    {
        throw std::runtime_error("Could not set tty to graphics mode");
        return;
    }

    focus_ = 1;

    vt_mode mode;
    mode.mode = VT_PROCESS;
    mode.acqsig = SIGUSR1;
    mode.relsig = SIGUSR2;

    if(ioctl(fd_, VT_SETMODE, &mode) == -1)
    {
        throw std::runtime_error("Could not set vt_mode");
        return;
    }

    //sig handler
    struct sigaction action;
    action.sa_handler = ttySignalhandler;

    sigaction(SIGUSR1, &action, nullptr);
    sigaction(SIGUSR2, &action, nullptr);
}

ttyHandler::~ttyHandler()
{
    activate();

    //dont care for exceptions, just try
    vt_mode mode;
    mode.mode = VT_AUTO;
    ioctl(fd_, VT_SETMODE, &mode);

    ioctl(fd_, KDSETMODE, KD_TEXT);

    //close fd
    close(fd_);
}

bool ttyHandler::activate()
{
    if(focus_) return 1;

    if(ioctl(fd_, VT_ACTIVATE, number_) == -1) return 0;
    if(ioctl(fd_, VT_WAITACTIVE, number_) == -1) return 0;

    focus_ = 1;
    return 1;
}

void ttyHandler::enteredTTY()
{
    beforeEnter_();

    ioctl(fd_, VT_RELDISP, VT_ACKACQ);
    focus_ = 1;

    afterEnter_();
}

void ttyHandler::leftTTY()
{
    beforeLeave_();

    ioctl(fd_, VT_RELDISP, 1); //allowed
    focus_ = 0;

    afterLeave_();
}
