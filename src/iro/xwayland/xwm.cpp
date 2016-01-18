#include <iro/xwayland/xwm.hpp>
#include <iro/compositor/compositor.hpp>

#include <nytl/log.hpp>

#include <wayland-server-core.h>

#include <xcb/composite.h>
#include <xcb/xfixes.h>
#include <xcb/xcbext.h>

#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstddef>

#include <cerrno>
#include <cstring>
#include <string>
#include <stdexcept>

namespace iro
{

//utility
namespace
{

enum atomName 
{
   WL_SURFACE_ID,
   WM_DELETE_WINDOW,
   WM_TAKE_FOCUS,
   WM_PROTOCOLS,
   WM_NORMAL_HINTS,
   MOTIF_WM_HINTS,
   UTF8_STRING,
   CLIPBOARD,
   CLIPBOARD_MANAGER,
   WM_S0,
   NET_WM_S0,
   NET_WM_PID,
   NET_WM_NAME,
   NET_WM_STATE,
   NET_WM_STATE_FULLSCREEN,
   NET_WM_STATE_MODAL,
   NET_WM_STATE_ABOVE,
   NET_SUPPORTED,
   NET_SUPPORTING_WM_CHECK,
   NET_WM_WINDOW_TYPE,
   NET_WM_WINDOW_TYPE_DESKTOP,
   NET_WM_WINDOW_TYPE_DOCK,
   NET_WM_WINDOW_TYPE_TOOLBAR,
   NET_WM_WINDOW_TYPE_MENU,
   NET_WM_WINDOW_TYPE_UTILITY,
   NET_WM_WINDOW_TYPE_SPLASH,
   NET_WM_WINDOW_TYPE_DIALOG,
   NET_WM_WINDOW_TYPE_DROPDOWN_MENU,
   NET_WM_WINDOW_TYPE_POPUP_MENU,
   NET_WM_WINDOW_TYPE_TOOLTIP,
   NET_WM_WINDOW_TYPE_NOTIFICATION,
   NET_WM_WINDOW_TYPE_COMBO,
   NET_WM_WINDOW_TYPE_DND,
   NET_WM_WINDOW_TYPE_NORMAL,
   ATOM_LAST
};

int openSocket(const sockaddr_un& addr, size_t path_size)
{
	int fd;
	socklen_t size = offsetof(sockaddr_un, sun_path) + path_size + 1;
	std::string path = (addr.sun_path[0] == '\0') ? 
		std::string("\\0") + std::string((addr.sun_path + 1)) : std::string(addr.sun_path);

	if((fd = socket(PF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0)) < 0)
	{
		nytl::sendWarning("XWM: failed to create socket ", path, "; errno: ", errno);
		return -1;
	}

	//in case someone forgot to clear it up
	unlink(addr.sun_path);

	if(bind(fd, reinterpret_cast<const sockaddr*>(&addr), size) < 0)
	{
		nytl::sendWarning("XWM: failed to bind socket to ", path, "; errno: ", errno);
		close(fd);
		return -1;
	}

   if(listen(fd, 1) < 0)
   {
	   nytl::sendWarning("XWM: failed to listen to socket at ", path, "; errno: ", errno);
	   unlink(addr.sun_path);
	   close(fd);
	   return -1;
   }

   return fd;
}


}

//XWM
struct XWindowManager::ListenerPOD
{
	wl_listener listener;
	XWindowManager* xwm;
};

int XWindowManager::sigusrHandler(int, void* data)
{
	nytl::sendLog("XWM: sigusr1 - XWayland init has finished");

	if(data) static_cast<XWindowManager*>(data)->initWM();
	return 1;
}

void XWindowManager::clientDestroyedHandler(struct wl_listener* listener, void*)
{
	nytl::sendLog("client destroyed...");

	XWindowManager::ListenerPOD* pod;
	pod = wl_container_of(listener, pod, listener);	

	if(!pod || !pod->xwm)
	{
		nytl::sendWarning("XWM: clientDestroy notify with invalid listener data");
		return;	
	}	

	pod->xwm->clientDestroyed();
}

//XWM implementation
XWindowManager::XWindowManager(Compositor& comp, Seat& seat) : compositor_(&comp), seat_(&seat)
{
	openDisplay();
	
	//xwayland to wayland
	//xwayland prints error if we use SOCK_CLOEXEC here. XXX?
	if(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, wlSocks) != 0)
	//if(socketpair(AF_UNIX, SOCK_STREAM, 0, wlSocks) != 0)
	{
		throw std::runtime_error("X11WM: failed to create wl socketpair");
	}

	//x to xwayland
	if(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, xSocks) != 0)
	{
		throw std::runtime_error("X11WM: failed to create x socketpair");
	}
	
	pid_t pid = fork();
	if(pid < 0)
	{
		throw std::runtime_error("X11WM: fork failed");
	}
	else if(pid == 0)
	{
		//child 
		executeXWayland();
		_exit(EXIT_FAILURE);
	}

	nytl::sendLog("XWM: fork succesful");

	//close child fds
	close(wlSocks[1]);
	close(xSocks[1]);
	close(socks[0]);
	close(socks[1]);

	wlSocks[1] = xSocks[1] = socks[0] = socks[1] = 0;

	//create screen client
	screenClient_ = wl_client_create(&compositor().wlDisplay(), wlSocks[0]);
	if(!screenClient_)
	{
		throw std::runtime_error("X11WM: failed to create client");
	}

	destroyListener_.reset(new ListenerPOD);
	destroyListener_->listener.notify = XWindowManager::clientDestroyedHandler;
	destroyListener_->xwm = this;
	wl_client_add_destroy_listener(screenClient_, &destroyListener_->listener);

	//sigusr signal handler
	signalEventSource_ = wl_event_loop_add_signal(&compositor().wlEventLoop(), 
				SIGUSR1, &XWindowManager::sigusrHandler, this );
}

XWindowManager::~XWindowManager()
{
	destroyWM();

	if(signalEventSource_)
	{
		wl_event_source_remove(signalEventSource_);
		signalEventSource_ = nullptr;
	}
	if(destroyListener_)
	{
		wl_list_remove(&destroyListener_->listener.link);
		destroyListener_.reset();
	}
	if(screenClient_) 
	{
		wl_client_destroy(screenClient_);
		screenClient_ = nullptr;
	}

	closeDisplay();
}

void XWindowManager::openDisplay()
{
	int lockFd = -1;
	unsigned int dpy = -1;
	std::string lockFile;

	//query on which display numbers X server are actually alive and choose the lowest free
	//display number for the xwayland server.
retry:
	dpy += 1;
	for(lockFd = -1; dpy <= 32 && lockFd < 0; ++dpy)
	{
		lockFile = "/tmp/.X" + std::to_string(dpy) + "-lock";

		//create new file
		if((lockFd = open(lockFile.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0444)) >= 0)
		{
			nytl::sendLog("XWM: succesfully created ", lockFile);
			break;
		}

		//open existing file
		if((lockFd = open(lockFile.c_str(),  O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC)) < 0)
		{
			nytl::sendLog("XWM: failed to open ", lockFile, " trying next display");
			continue;
		}
		
		char pid[12];
		std::memset(pid, 0, sizeof(pid));
		ssize_t bytes = read(lockFd, pid, sizeof(pid) - 1);

		close(lockFd);
		lockFd = -1;

		if(bytes != sizeof(pid) - 1)
		{
			nytl::sendLog("XWM: failed to read pid from ", lockFile);
			continue;
		}

		pid_t owner = 0;
		try
		{
			owner = std::stoi(pid);
		}
		catch(const std::invalid_argument& err)
		{
			nytl::sendLog("XWM: failed to convert pid string ", pid, " read from ", lockFile);
			continue;
		}

		//check if owner is still alive, if not so delete files
		errno = 0;
		if (kill(owner, 0) != 0 && errno == ESRCH) 
		{
			unlink(lockFile.c_str());
			
			std::string socketFile = "/tmp/.X11-unix/X" + std::to_string(dpy);
			unlink(socketFile.c_str());

			//try to create it again. if failing continue
			lockFd = open(lockFile.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0444);
			if(lockFd >= 0)
				break;
		}
	}	

	if(dpy > 32)
	{
		throw std::runtime_error("X11Wm: failed to open x11 display. Failed to open lock file");
		return;
	}

	//write pid to lock file
	std::string pidStr = std::to_string(getpid());
	if(write(lockFd, pidStr.c_str(), pidStr.size() - 1) != long(pidStr.size() - 1))
	{
		unlink(lockFile.c_str());
		close(lockFd);
		goto retry;	
	}	

	close(lockFd);

	//setup socket
	struct sockaddr_un addr;
	addr.sun_family = AF_LOCAL;
	addr.sun_path[0] = '\0';

	const char sockFormat[] = "/tmp/.X11-unix/X%d"; 
	size_t pathSize = std::snprintf(addr.sun_path + 1, sizeof(addr.sun_path) - 1, sockFormat, dpy);
	if((socks[0] = openSocket(addr, pathSize)) < 0) 
	{
		unlink(lockFile.c_str());
		unlink(addr.sun_path + 1);
		goto retry;
    }

	mkdir("/tmp/.X11-unix", 0777);
	pathSize = std::snprintf(addr.sun_path, sizeof(addr.sun_path), sockFormat, dpy);
	if((socks[1] = openSocket(addr, pathSize + 1)) < 0) 
	{
		close(socks[0]);
        unlink(lockFile.c_str());
        unlink(addr.sun_path);
		goto retry;
    }

	display_ = dpy;
	displayName_ = ":" + std::to_string(dpy);
}

void XWindowManager::closeDisplay()
{
	if(displayName_ == "-") return;

	if(xSocks[0] > 0) close(xSocks[0]);
	if(wlSocks[0] > 0) close(wlSocks[0]);

	std::string socket = "/tmp/.X11-unix/X" + std::to_string(display_);
	std::string lock = "/tmp/.X" + std::to_string(display_) + "-lock";
	unlink(socket.c_str());
	unlink(lock.c_str());

	display_ = 0;
	displayName_ = "-";
}

void XWindowManager::executeXWayland()
{
	//remove the O_CLOEXEC
	if(fcntl(wlSocks[1], F_SETFD, 0) != 0 ||
		fcntl(xSocks[1], F_SETFD, 0) != 0 || 
		fcntl(socks[0], F_SETFD, 0) != 0 ||
		fcntl(socks[1], F_SETFD, 0) != 0)
	{
		nytl::sendWarning("XWM::executeXWayland: fcntl failed");
		return;
	}
	
	//use dup instead of fctnl with SET 0?
	//wlSocks[1] = dup(wlSocks[1]);
	//xSocks[1] = dup(xSocks[1]);
	//socks[0] = dup(socks[0]);
	//socks[1] = dup(socks[1]);
	
	//if the signal handler for sigusr1 is sig_ign, the xserver will send a sigusr1 signal
	//to its parent process after initializing, we need this for further setup
	struct sigaction action;
	action.sa_handler = SIG_IGN;
	if(sigaction(SIGUSR1, &action, nullptr) != 0) 
	{
		nytl::sendWarning("XWM:executeXWayland: failed to set ignore handler for sigusr1");
		return;
	}

	//clear environment and set only needed vars
	const char *xdg_runtime = getenv("XDG_RUNTIME_DIR");
	if(!xdg_runtime) 
	{
		nytl::sendWarning("XWM::executeXWayland: failed to get XDG_RUNTIME_DIR env");
		return;
	}

	if(clearenv() != 0)
	{
		nytl::sendWarning("XWM::executeXWayland: failed to clear environment");
		return;
	}

	setenv("XDG_RUNTIME_DIR", xdg_runtime, 1);
	setenv("WAYLAND_SOCKET", std::to_string(wlSocks[1]).c_str(), 1);

	//redirect stdour/stderr
	//how about nytl::sendLog?
	//freopen("/dev/null", "w", stdout);
	//freopen("/dev/null", "w", stderr);

	//execute the xwayland server process     
	nytl::sendLog("XWM XWayland fork: starting XWayland on ", displayName_);
	execlp("Xwayland", "Xwayland",
			displayName_.c_str(),
            "-rootless",
            "-terminate",
            "-listen", std::to_string(socks[0]).c_str(),
            "-listen", std::to_string(socks[1]).c_str(),
            "-wm", std::to_string(xSocks[1]).c_str(),
            nullptr);

	nytl::sendLog("XWM XWayland fork: XWayland returned");
}

void XWindowManager::clientDestroyed()
{
	//x server exited (crashed)
	//try to restart it
	
	destroyListener_.reset();
	screenClient_ = nullptr;
}

void XWindowManager::initWM()
{
	if(signalEventSource_)
	{
		wl_event_source_remove(signalEventSource_);
		signalEventSource_ = nullptr;
	}

	setenv("DISPLAY", displayName_.c_str(), 1); //override it?.

	xConnection_ = xcb_connect_to_fd(xSocks[0], nullptr);
	if(xcb_connection_has_error(xConnection_))
	{
	}

   	struct 
	{
      std::string name;
      atomName atom;
   	} atmMap[ATOM_LAST] = {
		{ "WL_SURFACE_ID", WL_SURFACE_ID },
		{ "WM_DELETE_WINDOW", WM_DELETE_WINDOW },
		{ "WM_TAKE_FOCUS", WM_TAKE_FOCUS },
		{ "WM_PROTOCOLS", WM_PROTOCOLS },
		{ "WM_NORMAL_HINTS", WM_NORMAL_HINTS },
		{ "_MOTIF_WM_HINTS", MOTIF_WM_HINTS },
		{ "UTF8_STRING", UTF8_STRING },
		{ "CLIPBOARD", CLIPBOARD },
		{ "CLIPBOARD_MANAGER", CLIPBOARD_MANAGER },
		{ "WM_S0", WM_S0 },
		{ "_NET_WM_CM_S0", NET_WM_S0 },
		{ "_NET_WM_PID", NET_WM_PID },
		{ "_NET_WM_NAME", NET_WM_NAME },
		{ "_NET_WM_STATE", NET_WM_STATE },
		{ "_NET_WM_STATE_FULLSCREEN", NET_WM_STATE_FULLSCREEN },
		{ "_NET_WM_STATE_MODAL", NET_WM_STATE_MODAL },
		{ "_NET_WM_STATE_ABOVE", NET_WM_STATE_ABOVE },
		{ "_NET_SUPPORTED", NET_SUPPORTED },
		{ "_NET_SUPPORTING_WM_CHECK", NET_SUPPORTING_WM_CHECK },
		{ "_NET_WM_WINDOW_TYPE", NET_WM_WINDOW_TYPE },
		{ "_NET_WM_WINDOW_TYPE_DESKTOP", NET_WM_WINDOW_TYPE_DESKTOP },
		{ "_NET_WM_WINDOW_TYPE_DOCK", NET_WM_WINDOW_TYPE_DOCK },
		{ "_NET_WM_WINDOW_TYPE_TOOLBAR", NET_WM_WINDOW_TYPE_TOOLBAR },
		{ "_NET_WM_WINDOW_TYPE_MENU", NET_WM_WINDOW_TYPE_MENU },
		{ "_NET_WM_WINDOW_TYPE_UTILITY", NET_WM_WINDOW_TYPE_UTILITY },
		{ "_NET_WM_WINDOW_TYPE_SPLASH", NET_WM_WINDOW_TYPE_SPLASH },
		{ "_NET_WM_WINDOW_TYPE_DIALOG", NET_WM_WINDOW_TYPE_DIALOG },
		{ "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU", NET_WM_WINDOW_TYPE_DROPDOWN_MENU },
		{ "_NET_WM_WINDOW_TYPE_POPUP_MENU", NET_WM_WINDOW_TYPE_POPUP_MENU },
		{ "_NET_WM_WINDOW_TYPE_TOOLTIP", NET_WM_WINDOW_TYPE_TOOLTIP },
		{ "_NET_WM_WINDOW_TYPE_NOTIFICATION", NET_WM_WINDOW_TYPE_NOTIFICATION },
		{ "_NET_WM_WINDOW_TYPE_COMBO", NET_WM_WINDOW_TYPE_COMBO },
		{ "_NET_WM_WINDOW_TYPE_DND", NET_WM_WINDOW_TYPE_DND },
		{ "_NET_WM_WINDOW_TYPE_NORMAL", NET_WM_WINDOW_TYPE_NORMAL },
	};

	xcb_intern_atom_cookie_t atmCookies[ATOM_LAST];
	for(auto& atm : atmMap)
	{
		atmCookies[atm.atom] = 
			xcb_intern_atom(xConnection_, 0, atm.name.length(), atm.name.c_str());
	}

	const xcb_setup_t* setup = xcb_get_setup(xConnection_);
	xcb_screen_iterator_t screenIt = xcb_setup_roots_iterator(setup);
	xScreen_ = screenIt.data;

	uint32_t vals[] = 
	{ 
		XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | 
		XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | 
		XCB_EVENT_MASK_PROPERTY_CHANGE
	};

 	xcb_change_window_attributes_checked(xConnection_, xScreen_->root, XCB_CW_EVENT_MASK, vals);
	xFixes_ = xcb_get_extension_data(xConnection_, &xcb_xfixes_id);
	if(!xFixes_ || !xFixes_->present)
	{
	}

	xcb_xfixes_query_version_reply_t* xfixesReply;
	xfixesReply = xcb_xfixes_query_version_reply(xConnection_, 
		xcb_xfixes_query_version(xConnection_, XCB_XFIXES_MAJOR_VERSION, 
		XCB_XFIXES_MINOR_VERSION), nullptr);

   free(xfixesReply);

   const xcb_query_extension_reply_t* compositeExtension;
   compositeExtension = xcb_get_extension_data(xConnection_, &xcb_composite_id);
   if(!compositeExtension || !compositeExtension->present)
   {
   }

   xcb_composite_redirect_subwindows_checked(xConnection_, xScreen_->root, 
		   XCB_COMPOSITE_REDIRECT_MANUAL);

	for(auto& atm : atmMap) 
	{
		xcb_generic_error_t* error;
		xcb_intern_atom_reply_t* atomReply = xcb_intern_atom_reply(xConnection_, 
				atmCookies[atm.atom], &error);

		if(atomReply && !error)
			xAtoms_[atm.atom] = atomReply->atom;

		if(atomReply) free(atomReply);
	}

	xWindow_ = xcb_generate_id(xConnection_);

	constexpr unsigned int flags[] = {XCB_EVENT_MASK_PROPERTY_CHANGE};
	xcb_create_window_checked(xConnection_, XCB_COPY_FROM_PARENT, xWindow_, xScreen_->root, 0, 0, 
		1, 1, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT, xScreen_->root_visual, XCB_CW_EVENT_MASK, flags);


	xcb_atom_t supported[] = {
		xAtoms_[NET_WM_S0],
  	    xAtoms_[NET_WM_PID],
  	    xAtoms_[NET_WM_NAME],
  	    xAtoms_[NET_WM_STATE],
  	    xAtoms_[NET_WM_STATE_FULLSCREEN],
  	    xAtoms_[NET_SUPPORTING_WM_CHECK],
  	    xAtoms_[NET_WM_WINDOW_TYPE],
  	    xAtoms_[NET_WM_WINDOW_TYPE_DESKTOP],
  	    xAtoms_[NET_WM_WINDOW_TYPE_DOCK],
  	    xAtoms_[NET_WM_WINDOW_TYPE_TOOLBAR],
  	    xAtoms_[NET_WM_WINDOW_TYPE_MENU],
  	    xAtoms_[NET_WM_WINDOW_TYPE_UTILITY],
  	    xAtoms_[NET_WM_WINDOW_TYPE_SPLASH],
  	    xAtoms_[NET_WM_WINDOW_TYPE_DIALOG],
  	    xAtoms_[NET_WM_WINDOW_TYPE_DROPDOWN_MENU],
  	    xAtoms_[NET_WM_WINDOW_TYPE_POPUP_MENU],
  	    xAtoms_[NET_WM_WINDOW_TYPE_TOOLTIP],
  	    xAtoms_[NET_WM_WINDOW_TYPE_NOTIFICATION],
  	    xAtoms_[NET_WM_WINDOW_TYPE_COMBO],
  	    xAtoms_[NET_WM_WINDOW_TYPE_DND],
		xAtoms_[NET_WM_WINDOW_TYPE_NORMAL],
  	};

	constexpr std::size_t supSize = sizeof(supported)/sizeof(*supported);

  	xcb_change_property_checked(xConnection_, XCB_PROP_MODE_REPLACE, xScreen_->root,
		xAtoms_[NET_SUPPORTED], XCB_ATOM_ATOM, 32, supSize, supported);
  	xcb_change_property_checked(xConnection_, XCB_PROP_MODE_REPLACE, xScreen_->root,
		xAtoms_[NET_SUPPORTING_WM_CHECK], XCB_ATOM_WINDOW, 32, 1, &xWindow_);
  	xcb_change_property_checked(xConnection_, XCB_PROP_MODE_REPLACE, xWindow_,
		xAtoms_[NET_SUPPORTING_WM_CHECK], XCB_ATOM_WINDOW, 32, 1, &xWindow_);
  	xcb_change_property_checked(xConnection_, XCB_PROP_MODE_REPLACE, xWindow_,
		xAtoms_[NET_WM_NAME], xAtoms_[UTF8_STRING], 8, strlen("xiro"), "xiro");
  	xcb_set_selection_owner_checked(xConnection_, xWindow_, 
		xAtoms_[CLIPBOARD_MANAGER], XCB_CURRENT_TIME);
  	xcb_set_selection_owner_checked(xConnection_, xWindow_, xAtoms_[WM_S0],
		XCB_CURRENT_TIME);
  	xcb_set_selection_owner_checked(xConnection_, xWindow_, xAtoms_[NET_WM_S0],
		XCB_CURRENT_TIME);

  	uint32_t mask = XCB_XFIXES_SELECTION_EVENT_MASK_SET_SELECTION_OWNER |
  	                XCB_XFIXES_SELECTION_EVENT_MASK_SELECTION_WINDOW_DESTROY |
					XCB_XFIXES_SELECTION_EVENT_MASK_SELECTION_CLIENT_CLOSE;

	xcb_xfixes_select_selection_input_checked(xConnection_, xWindow_, xAtoms_[CLIPBOARD], mask);
		 
	xcb_flush(xConnection_);

	//event source
	xEventLoopSource_ = wl_event_loop_add_fd(&compositor().wlEventLoop(), xSocks[0], 
			WL_EVENT_READABLE, &XWindowManager::xEventLoopHandler, this);

	wl_event_source_check(xEventLoopSource_);
}

void XWindowManager::destroyWM()
{
	if(xCursor_)
	{
		xcb_free_cursor(xConnection_, xCursor_);
		xCursor_ = 0;
	}
	if(xWindow_)
	{
		xcb_destroy_window_checked(xConnection_, xWindow_);
		xWindow_ = 0;
	}
	if(xConnection_)
	{
		xcb_disconnect(xConnection_);
		xConnection_ = nullptr;
	}
}

int XWindowManager::xEventLoopHandler(int, unsigned int, void* data)
{
	if(data) return static_cast<XWindowManager*>(data)->xEventLoop();
	return 0;
}

unsigned int XWindowManager::xEventLoop()
{
	unsigned int count = 0;
	xcb_generic_event_t *event;

	while((event = xcb_poll_for_event(xConnection_))) 
	{
		switch(event->response_type & ~0x80)
		{
			case XCB_CREATE_NOTIFY:
			{
				break;
			}
		}

		free(event);
		count++;
	}

	xcb_flush(xConnection_);
	return count;
}

}
