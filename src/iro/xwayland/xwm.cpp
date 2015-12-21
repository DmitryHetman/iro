#include <iro/xwayland/xwm.hpp>

#include <nytl/log.hpp>

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
int openSocket(const sockaddr_un& addr, size_t path_size)
{
	int fd;
	socklen_t size = offsetof(sockaddr_un, sun_path) + path_size + 1;

	if((fd = socket(PF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0)) < 0)
	{
		nytl::sendWarning("XWM: failed to create socket ", addr.sun_path);
		return -1;
	}

	unlink(addr.sun_path);

	if(bind(fd, reinterpret_cast<const sockaddr*>(&addr), size) < 0)
	{
		nytl::sendWarning("XWM: failed to bind socket to ", addr.sun_path);
		close(fd);
		return -1;
	}

   if(listen(fd, 1) < 0)
   {
	   nytl::sendWarning("XWM: failed to listen to socket at ", addr.sun_path);
	   unlink(addr.sun_path);
	   close(fd);
	   return -1;
   }

   return fd;
}

namespace
{
	XWindowManager* globalWM_;
}

void sigusrHandler(int)
{
	nytl::sendLog("XWM: sigusr1 - XWayland init has finished");
	globalWM_->initWM();
}

//X11WM implementation
XWindowManager::XWindowManager(Compositor& comp, Seat& seat) : compositor_(&comp), seat_(&seat)
{
	openDisplay();
	
	//xwayland to wayland
	if(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, wlSocks) != 0)
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

	globalWM_ = this; //arrgh... do it with wayland loop?
	struct sigaction action;
	memset(&action, 0, sizeof(action));
	action.sa_handler = sigusrHandler;
	sigaction(SIGUSR1, &action, &savedSignalHandler_);
}

XWindowManager::~XWindowManager()
{
	closeDisplay();
}

void XWindowManager::openDisplay()
{
	int lockFd = -1;
	unsigned int dpy;
	std::string lockFile;

	//for loop
	for(dpy = 0; dpy <= 32 && lockFd < 0; ++dpy)
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
		
		throw std::runtime_error("X11WM: Failed to write pid to lock file.");
	}	

	close(lockFd);

	//setup socket
	struct sockaddr_un addr = { .sun_family = AF_LOCAL };
	addr.sun_path[0] = '\0';

	std::string socketFile = "/tmp/.X11-unix/X" + std::to_string(dpy);

	size_t pathSize = socketFile.size();
	std::strcpy(&addr.sun_path[1], socketFile.c_str());

	if((socks[0] = openSocket(addr, pathSize)) < 0) 
	{
		unlink(lockFile.c_str());
		unlink(addr.sun_path + 1);
		throw std::runtime_error("X11WM: Failed to open socket.");
    }

	mkdir("/tmp/.X11-unix", 0777);

	std::strcpy(&addr.sun_path[0], socketFile.c_str());
	pathSize++;
	if((socks[1] = openSocket(addr, pathSize)) < 0) 
	{
		close(socks[0]);
        unlink(lockFile.c_str());
        unlink(addr.sun_path);
		throw std::runtime_error("X11WM: Failed to open socket 2.");
    }

	displayName_ = ":" + std::to_string(dpy);
	display_ = dpy;
}

void XWindowManager::closeDisplay()
{
	if(displayName_ == "-") return;

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
		fcntl(socks[0], F_SETFD, 0) != 0)
	{
		nytl::sendWarning("XWM::executeXWayland: fcntl failed");
		return;
	}

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
	freopen("/dev/null", "w", stdout);
	freopen("/dev/null", "w", stderr);

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

void XWindowManager::initWM()
{
	sigaction(SIGUSR1, &savedSignalHandler_, nullptr);
	setenv("DISPLAY", displayName_.c_str(), 0); //dont override it.

	//init event loop and stuff
}

}
