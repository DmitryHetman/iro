#include <iro/util/fork.hpp>
#include <iro/compositor/compositor.hpp>
#include <ny/base/log.hpp>

#include <wayland-server-core.h>

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#include <cstdio>
#include <cstring>
#include <stdexcept>

namespace iro
{

///
bool Fork::alive() const
{
	return (kill(pid_, 0) != -1);
}

///
ForkHandler::ForkHandler(Compositor& compositor)
{
	//output redirection pipes
	int pipes[2];
	if(pipe(pipes) != 0)
	{
		throw std::runtime_error("Fork Handler: unable to create pipes for output redirection");
	}

	readOutputPipe_ = pipes[0];
	writeOutputPipe_ = pipes[1];

	fcntl(readOutputPipe_, F_SETFD, O_NONBLOCK);

	//add event source
	outputEventSource_ = wl_event_loop_add_fd(&compositor.wlEventLoop(), readOutputPipe_, 
			WL_EVENT_READABLE, &ForkHandler::outputHandler, this);

	if(!outputEventSource_)
	{
		throw std::runtime_error("ForkHandler: unable to init wlEventSource for output pipe");
	}
}

ForkHandler::~ForkHandler()
{
	if(outputEventSource_)
	{
		wl_event_source_remove(outputEventSource_);
		outputEventSource_ = nullptr;
	}

	if(readOutputPipe_)
	{
		close(readOutputPipe_);
		readOutputPipe_ = -1;
	}

	if(writeOutputPipe_)
	{
		close(writeOutputPipe_);
		writeOutputPipe_ = -1;
	}
}

int ForkHandler::outputHandler(int fd, unsigned int, void*)
{
	//where to redirect it?
	char buffer[512];
	int readBytes = 0;

	if((readBytes = read(fd, buffer, 512)) > 0)
	{
		buffer[readBytes] = '\0'; //needed?
		*ny::logLogger().stream << "Fork output: " << buffer;
	}

	return 1;	
}

void ForkHandler::throwErrorMessage(const std::string& err) const
{
	int error = 0;
	static const std::string errorMsg = "ForkHandler::exec: exec system call failed, ";

	try
	{
		error = std::stoi(err);
	}
	catch(const std::invalid_argument&)
	{
		ny::sendWarning("ForkHandler::exec: could not convert buffer into int");
		throw std::runtime_error(errorMsg + "unknown error");
	}

	if(error == ENOENT)
		throw std::logic_error(errorMsg + "ENOENT: invalid file");	

	if(error == ENAMETOOLONG)
		throw std::logic_error(errorMsg + "ENAMETOOLONG: filename is too long");

	if(error == EISDIR)
		throw std::logic_error(errorMsg + "EISDIR: filename is a directory");

	throw std::logic_error(errorMsg + "errno was set to " + err + ": " + std::strerror(error));
}

Fork ForkHandler::exec(const std::string& s, const std::vector<std::string>& args)
{
	//args
	std::vector<const char*> a(args.size() + 2);
	a[0] = s.c_str();
	for(std::size_t i(1); i < args.size() + 1; ++i)
	{
		a[i] = args[i - 1].c_str();
	}
	a.back() = nullptr;

	//create pipe
	int pipes[2];
	if(pipe(pipes) != 0)
	{
		throw std::runtime_error("ForkHandler::exec: failed to create communication pipes");
	}

	//fork
	pid_t pid = fork();
	if(pid == 0)
	{
		//close reading pipe, set cloexec for writing pipe
		close(pipes[1]);
		fcntl(pipes[0], F_SETFD, FD_CLOEXEC);

		//close reading output pipe
		close(readOutputPipe_);

		//redirect input? XXX
		//redirect output
		dup2(writeOutputPipe_, STDOUT_FILENO);
		dup2(writeOutputPipe_, STDERR_FILENO);

		//exec, will close (cloexec) the writing pipe if succesful
		execvp(s.c_str(), const_cast<char**>(a.data()));

		//if exec is succesful, this will never be executed
		write(pipes[0], &errno, sizeof(errno));
		_exit(EXIT_FAILURE);
	}
	else if(pid < 0)
	{
		//error
		throw std::runtime_error("ForkHandler::exec: failed to fork a child");
	}

	//close writing pipe
	close(pipes[0]);
	
	//read from reading pipe
	char buf[sizeof(errno) + 1];
	buf[sizeof(errno)] = '\n';

	int readBytes = read(pipes[1], buf, sizeof(errno));

	//close reading end
	close(pipes[1]);

	//check answer
	if(readBytes > 0) //there were bytes written to the pipe before closing -> exec failed
		throwErrorMessage(buf); //will throw

	Fork fork(pid);
	return fork;	
}

}
