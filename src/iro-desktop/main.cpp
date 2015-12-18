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

#include <nytl/log.hpp>
#include <nytl/make_unique.hpp>
#include <nytl/misc.hpp>

#include <wayland-server-core.h>

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <dlfcn.h>
#include <signal.h>

iro::ShellModule* loadShell(const std::string& name)
{
	void* handle = dlopen(name.c_str(), RTLD_LAZY);
	if(!handle)
	{
		nytl::sendWarning("dlopen cant find ", name);
		return nullptr;
	}

	using loadFunc = iro::ShellModule*(*)();
	loadFunc loader = (loadFunc) dlsym(handle, "iro_shell_module");
	if(!loader)
	{
		nytl::sendWarning("cant find iro_shell_module function in shell module");
		return nullptr;
	}

	return loader();
}

void intHandler(int)
{
	nytl::sendLog("received signal interrupt. exiting");
	exit(5);
}

int main()
{
	std::ofstream logStream;
	logStream.rdbuf()->pubsetbuf(0, 0);
	logStream.open("log.txt");
	try
	{

	nytl::sendLog.stream = &logStream;
	nytl::sendWarning.stream = &logStream;
	nytl::sendError.stream = &logStream;

	nytl::sendLog("Started Iro Desktop");

	iro::Compositor myCompositor;
	iro::Seat mySeat(myCompositor);

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
		myTerminalHandler.reset(new iro::TerminalHandler(myCompositor, *myDeviceHandler));
		myBackend.reset(new iro::KmsBackend(myCompositor, *myDeviceHandler));
		myUDevHandler.reset(new iro::UDevHandler(myCompositor));
		myInputHandler.reset(new iro::InputHandler(myCompositor, mySeat, 
					*myUDevHandler, *myDeviceHandler));

		static_cast<iro::KmsBackend*>(myBackend.get())->setCallbacks(*myTerminalHandler);
	}

	nytl::sendLog("finished backend setup");

	if(!myBackend)
	{
		nytl::sendError("no valid backend found");
		return 0;
	}

	myCompositor.backend(*myBackend);

	auto* myShell = loadShell("libiro-shell.so");
	if(!myShell)
	{
		nytl::sendError("failed to load shell module");
		return 0;
	}
	myShell->init(myCompositor, mySeat);

	for(auto* outp : myBackend->outputs())
		outp->onDraw(nytl::memberCallback(&iro::ShellModule::render, myShell));

	nytl::sendLog("starting main loop");
	myCompositor.run();
	nytl::sendLog("Finished Iro Desktop");
	*nytl::sendLog.stream << std::flush;

	}
	catch(const std::exception& err)
	{
		nytl::sendLog("Caught Exception: ", err.what());
	}

	*nytl::sendLog.stream << std::flush;
	return 1;	
}
