#include <iro/compositor/shell.hpp>
#include <iro/compositor/surface.hpp>
#include <iro/compositor/xdgShell.hpp>
#include <iro/backend/surfaceContext.hpp>
#include <iro/backend/output.hpp>
#include <iro/compositor/window.hpp>
#include <iro/seat/seat.hpp>
#include <iro/seat/pointer.hpp>
#include <iro/seat/keyboard.hpp>
#include <ny/draw/drawContext.hpp>
#include <ny/draw/image.hpp>
#include <ny/draw/gl/texture.hpp>
#include <ny/draw/gl/glad/glad.h>
using namespace iro;

#include <ny/base/log.hpp>

//myModule
class MyShellModule : public ShellModule
{
protected:
	Compositor* compositor_ = nullptr;
	Seat* seat_ = nullptr;

	std::unique_ptr<Shell> shell_;
	std::unique_ptr<XdgShell> xdgShell_;

	ny::GlTexture myTexture;
	ny::GlTexture cursorTexture;

	std::vector<Window*> windows_;
	Window* focus_;
	unsigned int highestZOrder_ = 0;

public:
	virtual ~MyShellModule();

	virtual void init(Compositor& comp, Seat& s) override;
	virtual void render(Output& outp, ny::DrawContext& dc) override;

	virtual void windowCreated(Window& win) override 
	{ 
		windows_.push_back(&win); 
		win.zOrder(highestZOrder_ + 1);
		highestZOrder_++;
		focus_ = &win;
	}
	virtual void windowDestroyed(Window& win) override 
	{
		auto it = std::find(windows_.begin(), windows_.end(), &win);
		if(it != windows_.end()) windows_.erase(it);
	}

	void keyboardFocus(SurfaceRes* old, SurfaceRes* newS);
};

//loadFunc
extern "C"
{

ShellModule* iro_shell_module()
{
	return new MyShellModule();
}

}

//impl
MyShellModule::~MyShellModule()
{
}

void MyShellModule::init(Compositor& comp, Seat& seat)
{
	compositor_ = &comp;
	seat_ = &seat;

	shell_.reset(new Shell(comp));
	xdgShell_.reset(new XdgShell(comp));

	seat.keyboard()->onFocus(nytl::memberCallback(&MyShellModule::keyboardFocus, this));
}

void MyShellModule::keyboardFocus(SurfaceRes* oldS, SurfaceRes* newS)
{
	/*
	if(old)
	{
		auto role = old->role();
		if(role)
		{
			auto windowRole = dynamic_cast<Window*>(role);
			//doSomething?
		}
	}
	*/

	focus_ = nullptr;

	if(newS)
	{
		auto role = newS->role();
		if(role)
		{
			auto windowRole = dynamic_cast<WindowSurfaceRole*>(role);
			if(windowRole)
			{
				windowRole->window().zOrder(highestZOrder_ + 1);
				std::cout << "order: " << windowRole->window().zOrder() << "\n";
				highestZOrder_++;
				focus_ = &windowRole->window();
			
				std::sort(windows_.begin(), windows_.end(), 
					[](Window* a, Window* b){ return a->zOrder() < b->zOrder(); });
			}
		}
	}

	std::cout << "yoyoyoyoyoyo " << oldS << " -> " << newS << "\n";
}

void MyShellModule::render(Output& outp, ny::DrawContext& dc)
{
	if(!myTexture.glTexture())
	{
		ny::Image myImage;
		ny::sendLog("loading wallpaper... ", myImage.load("wallpaper.png"));

		myTexture.create(myImage);
	}
	
	glViewport(0, 0, outp.size().x, outp.size().y);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	dc.clear(ny::Color::red);

	ny::TextureBrush texBrush;
	texBrush.extents.position = {0.f, 0.f};
	texBrush.extents.size = outp.size();
	texBrush.texture = &myTexture;

	ny::Rectangle bg = texBrush.extents;
	dc.mask(bg);
	dc.fill(texBrush);

	SurfaceRes* cursorSurface = nullptr;
	ny::sendLog("Shell: Drawing ", outp.mappedSurfaces().size(), " mapped surfaces");
	for(auto& surf : outp.mappedSurfaces())
	{
		if(surf->roleType() == surfaceRoleType::cursor)
			cursorSurface = surf;

		/*
		ny::Rectangle surfaceRect(surf->extents());
		surfaceRect.position(surfaceRect.position() - outp.extents().position);
		dc.mask(surfaceRect);

		ny::TextureBrush surfaceBrush;
		surfaceBrush.extents = surfaceRect;
		surfaceBrush.texture = surf->surfaceContext().content();

		dc.fill(surfaceBrush);
		*/

		surf->sendFrameDone();
	}

	//windows are sorted by z order
	for(auto& window : windows_)
	{
		auto* surf = window->surfaceRes();	
		if(!surf) continue;

		if(window == focus_)
		{
			ny::Rectangle surfaceRect(surf->extents());
			surfaceRect.position(surfaceRect.position() + nytl::Vec2ui{20,20});
			surfaceRect.size(surfaceRect.size() - nytl::Vec2ui{40,40});
			dc.mask(surfaceRect);
			dc.fill(ny::Color::green);
		}

		ny::Rectangle surfaceRect(surf->extents());
		surfaceRect.position(surfaceRect.position() - outp.extents().position);
		dc.mask(surfaceRect);

		ny::TextureBrush surfaceBrush;
		surfaceBrush.extents = surfaceRect;
		surfaceBrush.texture = surf->surfaceContext().content();

		dc.fill(surfaceBrush);
	}
	
	if(cursorSurface)
	{
		SurfaceRes* surf = cursorSurface;
		ny::Rectangle surfaceRect(surf->extents());
		surfaceRect.position(surfaceRect.position() - outp.extents().position);
		dc.mask(surfaceRect);

		ny::TextureBrush surfaceBrush;
		surfaceBrush.extents = surfaceRect;
		surfaceBrush.texture = surf->surfaceContext().content();

		dc.fill(surfaceBrush);

		surf->sendFrameDone();
	}
	
	else
	{
		if(!cursorTexture.glTexture())
		{
			ny::Image cursorImage;
			ny::sendLog("loading cursor image: ", cursorImage.load("cursor.png"));
			cursorTexture.create(cursorImage);
		}

		ny::TextureBrush cursorBrush;
		cursorBrush.extents.position = seat_->pointer()->position() - outp.extents().position;
		cursorBrush.extents.size = nytl::Vec2f(30, 30);
		cursorBrush.texture = &cursorTexture;

		dc.mask(ny::Rectangle(cursorBrush.extents.position, nytl::Vec2f(30, 30)));
		dc.fill(cursorBrush);
	}
}
