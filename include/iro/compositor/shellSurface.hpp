#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>
#include <iro/compositor/surface.hpp>
#include <iro/compositor/window.hpp>

#include <nytl/time.hpp>

namespace iro
{

//Abstract pinageable class for shellSurface and xdgShell, connect it to client 
//(e.g. Client::active())

///Represents a wayland wl_shell_surface resource.
class ShellSurfaceRes : public Resource, public Window
{
protected:
	SurfaceRes* surface_;

	unsigned int pingSerial_ = 0;
	nytl::TimePoint pingTime_;

	/*
	union
	{
		struct {} dummy_;

		struct
		{ 
			unsigned int method_;  
			unsigned int frames_;
			Output& output;
		} fullscreen_;

		struct
		{
			SurfaceRes* parent;
			nytl::vec2i position;
			unsigned int flags;
		} transient_;

		struct
		{
			SeatRes* seat;
			SurfaceRes* parent;
			nytl::vec2i position;
			unsigned int flags;		
		} popup_;

		struct
		{
			OutputRes* output;
		} maximized_;
	};
	*/

public:
	ShellSurfaceRes(SurfaceRes& surf, wl_client& client, unsigned int id);

	bool activePing() const { return (pingSerial_ != 0); }
	nytl::TimeDuration pingTime() const { return nytl::now() - pingTime_; }
	unsigned int ping();
	void pong(unsigned int);

	virtual SurfaceRes* surface() const { return surface_; }
	virtual void close() override {} //todo
};

///Represents the wayland surface role of a shell surface.
class ShellSurfaceRole : public WindowSurfaceRole
{
public:
	ShellSurfaceRole(ShellSurfaceRes& res) : WindowSurfaceRole(res) {};

    virtual unsigned int roleType() const override { return surfaceRoleType::shell; }
	ShellSurfaceRes& shellSurfaceRes() const { return static_cast<ShellSurfaceRes&>(window()); }
};

}
