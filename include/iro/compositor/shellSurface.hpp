#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>
#include <iro/compositor/surface.hpp>

#include <nytl/time.hpp>

namespace iro
{

///Represents a wayland wl_shell_surface resource.
class ShellSurfaceRes : public Resource
{
public:
	static nytl::timeDuration inactiveDuration;

protected:
	SurfaceRes& surface_;

	std::string title_;
	std::string class_;

	unsigned int pingSerial_ = 0;
	nytl::timePoint pingTime_;

	nytl::vec2i position_;

	unsigned int resizeEdges_;

	unsigned int state_;
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

	//
	void move(const nytl::vec2i& delta);
	void resize(const nytl::vec2i& delta);

public:
	ShellSurfaceRes(SurfaceRes& surf, wl_client& client, unsigned int id);

	void ping(unsigned int serial);
	void pong(unsigned int serial);
	void beginMove(SeatRes& seat, unsigned int serial);
	void beginResize(SeatRes& seat, unsigned int serial, unsigned int edges);

	void setToplevel();
	void setTransient(SurfaceRes& parent, const nytl::vec2i& pos, unsigned int flags);
	void setFullscreen(unsigned int mthd, unsigned int frames, OutputRes& output);
	void setPopup(SeatRes& seat, unsigned int serial, SurfaceRes& parent, 
			const nytl::vec2i& pos, unsigned int flags);
	void setMaximized(OutputRes& output);

	void className(const std::string& name){ class_ = name; }
	void title(const std::string& title){ title_ = title; }

	nytl::vec2i position() const { return position_; }
	bool inactive() const { return (!pingSerial_ || nytl::now() - pingTime_ < inactiveDuration ); }
};

///Represents the wayland surface role of a shell surface.
class ShellSurfaceRole : public SurfaceRole
{
protected:
	ShellSurfaceRes& shellSurfaceRes_;

public:
	ShellSurfaceRole(ShellSurfaceRes& res) : shellSurfaceRes_(res) {};

    virtual nytl::vec2i position() const override { return shellSurfaceRes_.position(); }
    virtual bool mapped() const override { return 1; } //todo?
    virtual void commit() override {} //todo
    virtual unsigned int roleType() const override { return surfaceRoleType::shell; }

	ShellSurfaceRes& shellSurfaceRes() const { return shellSurfaceRes_; }
};

}
