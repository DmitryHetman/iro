#pragma once

#include <iro/include.hpp>
#include <iro/compositor/surface.hpp>

#include <nytl/vec.hpp>
#include <nytl/rect.hpp>

#include <string>

namespace iro
{

///Commited/pending state of a window, containing its state and geometry.
class WindowState
{
public:

public:
	void reset(){}; //nothing to do here

public:
	nytl::rect2i geometry_ = {0, 0, 0, 0};
};

///Base class for all shell surface classes.
///Used e.g. by X11Surface, ShellSurfaceRes and XdgSurfaceRes
class Window
{
public:
	enum class State : unsigned int
	{
		normal = (1 << 0),
		maximized = (1 << 1),
		minimized = (1 << 2),
		fullscreen = (1 << 3),
		resizing = (1 << 4),
		activated = (1 << 5),
		transient = (1 << 6),
		popup = (1 << 7),
		moving = (1 << 8)
	};

protected:
	WindowState pending_ {};
	WindowState commited_ {};
	State states_ = State::normal;

	nytl::vec2i position_;
	nytl::vec2ui normalSize_; //size when in normal state...

	std::string title_;
	std::string appID_;

	Window* parent_ = nullptr; //needed here?

	//active union member correspunds  
	union
	{
		struct { Output* output; } maximized_;
		struct { Output* output; unsigned int method; } fullscreen_;
		struct { SurfaceRes* parent_; nytl::vec2i position; unsigned int flags; } transient_;
		struct { SurfaceRes* parent; nytl::vec2i position; unsigned int flags; Seat* seat; } popup_;
	};

protected:
	virtual void move(const nytl::vec2i& delta);
	virtual void resize(const nytl::vec2i& delta, unsigned int edges);
	virtual void sendConfigure(const nytl::vec2ui& size) const;
	void normalState();

public:
	State states() const { return states_; }

	nytl::rect2i geometry() const { return commited_.geometry_; }
	nytl::vec2i position() const { return position_; }
	nytl::vec2ui size() const { return geometry().size; }

	const std::string& title() const { return title_; }
	const std::string& appID() const { return appID_; }

	virtual void startMove(Seat& seat, const Event& trigger);
	virtual void startResize(Seat& seat, const Event& trigger, unsigned int edges);

	void startMove(Seat& seat, unsigned int triggerSerial);
	void startResize(Seat& seat, unsigned int triggerSerial, unsigned int edges);

	void showWindowMenu(Seat& seat, const Event& trigger, const nytl::vec2i& position);
	void showWindowMenu(Seat& seat, unsigned int triggerSerial, const nytl::vec2i& position);

	void geometry(const nytl::rect2i& geo);

	void title(const std::string& title){ title_ = title; }
	void appID(const std::string& id){ appID_ = id; }

	virtual void setMaximized(Output& outp);
	virtual void setFullscreen(Output& outp, unsigned int method);
	virtual void setMinimized();
	virtual void setTransient(SurfaceRes& parent, const nytl::vec2i& position, unsigned int flags);
	virtual void setPopup(SurfaceRes& parent, const nytl::vec2i& pos, unsigned int flags, Seat&);
	virtual void setNormal();

	virtual bool mapped() { return (!parent_ || parent_->mapped()); }
	virtual void commit();
	virtual void close() = 0;

	virtual SurfaceRes* surfaceRes() const { return nullptr; }
};

///Role base class for wl_surfaces that have a window role (e.g. wl_shell_surface or xdg_surface)
class WindowSurfaceRole : public SurfaceRole
{
protected:
	Window* window_ = nullptr;

public:
	WindowSurfaceRole(Window& w) : window_(&w) {}

    virtual nytl::vec2i position() const override { return window_->position(); }
    virtual bool mapped() const override { return window_->mapped(); }
    virtual void commit() override { window_->commit(); }

	Window& window() const { return *window_; }
};

}
