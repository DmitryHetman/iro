#include <iro/compositor/compositor.hpp>
#include <iro/compositor/shell.hpp>
#include <iro/backend/x11.hpp>
#include <iro/backend/kms.hpp>
#include <iro/backend/egl.hpp>
#include <iro/backend/devices.hpp>
#include <iro/backend/tty.hpp>
#include <iro/backend/input.hpp>
#include <iro/backend/udev.hpp>
#include <iro/backend/logind.hpp>
#include <iro/backend/dbus.hpp>
#include <iro/seat/seat.hpp>
#include <iro/seat/keyboard.hpp>
#include <iro/util/fork.hpp>
#include <iro/xwayland/xwm.hpp>

#include <ny/base/log.hpp>
#include <nytl/misc.hpp>
#include <nytl/enumOps.hpp>

#include <wayland-server-core.h>

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <dlfcn.h>
#include <signal.h>
#include <linux/input.h>

int vtSwitchTo = 0;
wl_event_source* idleSwitchSource = nullptr;

iro::ShellModule* loadShell(const std::string& name)
{
	void* handle = dlopen(name.c_str(), RTLD_LAZY);
	if(!handle)
	{
		ny::sendWarning("dlopen cant find ", name);
		return nullptr;
	}

	using loadFunc = iro::ShellModule*(*)();
	loadFunc loader = (loadFunc) dlsym(handle, "iro_shell_module");
	if(!loader)
	{
		ny::sendWarning("cant find iro_shell_module function in shell module");
		return nullptr;
	}

	return loader();
}

void intHandler(int)
{
	ny::sendLog("received signal interrupt. exiting");
	exit(5);
}

void idleSwitch(void* data)
{
	if(!idleSwitchSource || !data || !vtSwitchTo) return;

	auto* th = static_cast<iro::TerminalHandler*>(data);
	th->activate(vtSwitchTo);
	vtSwitchTo = 0;

	wl_event_source_remove(idleSwitchSource);
	idleSwitchSource = 0;
}

std::ofstream logStream;

int main()
{
	char buffer[64];

	//logStream.rdbuf()->pubsetbuf(buffer, sizeof(buffer));
	logStream.rdbuf()->pubsetbuf(nullptr, 0);
	logStream.open("log.txt");
	try
	{

	ny::logLogger().stream = &logStream;
	ny::warningLogger().stream = &logStream;
	ny::errorLogger().stream = &logStream;

	ny::sendLog("Started Iro Desktop");

	iro::Compositor myCompositor;
	iro::Seat mySeat(myCompositor);
	iro::ForkHandler myForkHandler(myCompositor);

	std::unique_ptr<iro::DBusHandler> myDBusHandler = nullptr;
	std::unique_ptr<iro::LogindHandler> myLogindHandler = nullptr;
	std::unique_ptr<iro::DeviceHandler> myDeviceHandler = nullptr;
	std::unique_ptr<iro::Backend> myBackend = nullptr;
	std::unique_ptr<iro::TerminalHandler> myTerminalHandler = nullptr;
	std::unique_ptr<iro::UDevHandler> myUDevHandler = nullptr;
	std::unique_ptr<iro::InputHandler> myInputHandler = nullptr;

	if(iro::X11Backend::available())
	{
		iro::X11Backend* xbcknd = new iro::X11Backend(myCompositor, mySeat);
		myBackend.reset(xbcknd);
		xbcknd->createOutput();
	}
	else
	{
		myDBusHandler.reset(new iro::DBusHandler(myCompositor));
		myLogindHandler.reset(new iro::LogindHandler(*myDBusHandler));
		myDeviceHandler.reset(new iro::DeviceHandler(*myDBusHandler, *myLogindHandler));
		myTerminalHandler.reset(new iro::TerminalHandler(myCompositor));
		myBackend.reset(new iro::KmsBackend(myCompositor, *myDeviceHandler));
		myUDevHandler.reset(new iro::UDevHandler(myCompositor));
		myInputHandler.reset(new iro::InputHandler(myCompositor, mySeat, 
					*myUDevHandler, *myDeviceHandler));

		static_cast<iro::KmsBackend*>(myBackend.get())->setCallbacks(*myTerminalHandler);
		myLogindHandler->onActive([&](bool b)
				{
					ny::sendLog("active: ", b);
					if(b)
					{
						myInputHandler->resume(); 
						//myTerminalHandler->activate(myTerminalHandler->number());
						//myTerminalHandler->waitActive(myTerminalHandler->number());
					}
					else 
					{
						myInputHandler->suspend();
					}
				});
		
		std::cout << (int)mySeat.keyboard()->modifiers() << "\n"; 
		std::cout << (int)(iro::Keyboard::Modifier::ctrl | iro::Keyboard::Modifier::alt) << "\n"; 

		if(mySeat.keyboard())
		{
			mySeat.keyboard()->onKey([&](unsigned int key, bool pressed)
					{ 
						if(!pressed) return;

						if(mySeat.keyboard()->modifiers() != 
								(iro::Keyboard::Modifier::ctrl | iro::Keyboard::Modifier::alt))
							return;

						if(key == KEY_Q)
						{
							idleSwitchSource = wl_event_loop_add_idle(&myCompositor.wlEventLoop(), 
									idleSwitch, myTerminalHandler.get());
							vtSwitchTo = myTerminalHandler->number() - 1;
						}	
						else if(key == KEY_E)
						{
							idleSwitchSource = wl_event_loop_add_idle(&myCompositor.wlEventLoop(), 
									idleSwitch, myTerminalHandler.get());
							vtSwitchTo = myTerminalHandler->number() + 1;
						}
					});
		}
	}

	if(mySeat.keyboard())
	{

		mySeat.keyboard()->onKey([&](unsigned int key, bool pressed)
			{ 
				if(mySeat.keyboard()->modifiers() != iro::Keyboard::Modifier::ctrl) return;
				if(pressed && key == KEY_T)
				{
					ny::sendLog("starting weston terminal");
					myForkHandler.exec("weston-terminal", {"--shell=/bin/bash"});
				}
			});
	}

	ny::sendLog("finished backend setup");


	ny::sendLog("set up x window manager");

	if(!myBackend)
	{
		ny::sendError("no valid backend found");
		return 0;
	}

	myCompositor.backend(*myBackend);

	auto* myShell = loadShell("libiro-shell.so");
	if(!myShell)
	{
		ny::sendError("failed to load shell module");
		return 0;
	}
	myShell->init(myCompositor, mySeat);
	myCompositor.shell(*myShell);

	
	for(auto* outp : myBackend->outputs())
		outp->onDraw(nytl::memberCallback(&iro::ShellModule::render, myShell));
	
	auto xwm = std::make_unique<iro::XWindowManager>(myCompositor, mySeat);

	ny::sendLog("starting main loop");
	//myCompositor.run(nytl::seconds(30));
	myCompositor.run();
	ny::sendLog("Finished Iro Desktop");
	*ny::logLogger().stream << std::flush;

	}
	catch(const std::exception& err)
	{
		ny::sendLog("Caught Exception: ", err.what());
	}

	*ny::logLogger().stream << "iro main extited normally. " << std::flush;
	return 1;	
}
