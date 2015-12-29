#pragma once

#include <iro/include.hpp>
#include <nytl/nonCopyable.hpp>
#include <nytl/callback.hpp>

#include <string>
#include <vector>

namespace iro
{

//TODO: individual output redirection for each fork, so it is known which fork send which
//output. If each fork has its own pipes, output/input callbacks for forks can be implemented as 
//well.

///Represents a fork. Can be used to check wheter a fork is still alive.
class Fork
{
protected:
	unsigned int pid_;

public:
	Fork(unsigned int pid) : pid_(pid) {}

	unsigned int pid() const { return pid_; }
	bool alive() const;
};

///Responsible for creating and managing forks/exec child processes.
class ForkHandler : public nytl::nonCopyable
{
protected:
	static int outputHandler(int, unsigned int, void*);

protected:
	int readOutputPipe_ = -1;
	int writeOutputPipe_ = -1;
	wl_event_source* outputEventSource_ = nullptr;

protected:
	void throwErrorMessage(const std::string& error) const;

public:
	///Constructs the ForkHandler with a given Compositor object which is used for registering
	///the ForkHandlers event sources.
	///Throws std::runtime_error if unable to create the pipes used for fork output redirection.
	ForkHandler(Compositor&);
	~ForkHandler();

	///Tries to fork the process and execute the given program with the given args. 
	///Effectively calls fork and then execvp with the program as first and second parameter, 
	///followed by the args. 
	///If forking and executing were both succesful, it returns a reference to the newly created
	///fork object which can be used to check the status of the fork.
	///If fork() fails it will throw a runtime error. If exec() fails, it will throw a runtime
	///or logic error depending on the failing reason.
	Fork exec(const std::string& program, const std::vector<std::string>& args);
};

}
