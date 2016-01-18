#pragma once

#include <iro/include.hpp>
#include <iro/util/global.hpp>

namespace ny
{
	class DrawContext;
}

namespace iro
{

///The ShellModule class represents a dynamic loadable shell module which is resposible of
///rendering all outputs and surfaces. A shell module has to provide its own shell and shell
///surface impelementations but they can use the helper base classes shell and shellSurface.
class ShellModule
{
public:
	virtual void init(Compositor& comp, Seat& s) = 0;
	virtual void render(Output&, ny::DrawContext& dc) = 0;

	virtual void windowCreated(Window&) {}
	virtual void windowDestroyed(Window&) {}
};

///The Shell class represents a wl_shell and can be used as base class for shell implementations
///of ShellModules.
class Shell : public Global
{
protected:
	Compositor* compositor_;	

public:
	Shell(Compositor& comp);
	virtual ~Shell() = default;

	virtual void getShellSurface(SurfaceRes& surface, unsigned int id);

	Compositor& compositor() const { return *compositor_; }
};


}
