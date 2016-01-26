#include <iro/compositor/shell.hpp>
#include <iro/compositor/surface.hpp>
#include <iro/compositor/xdgShell.hpp>
#include <iro/backend/surfaceContext.hpp>
#include <iro/backend/output.hpp>
#include <iro/seat/seat.hpp>
#include <iro/seat/pointer.hpp>
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

	unsigned int windowCount_ {0};

public:
	virtual ~MyShellModule();

	virtual void init(Compositor& comp, Seat& s) override;
	virtual void render(Output& outp, ny::DrawContext& dc) override;

	virtual void windowCreated(Window& win) override { windowCount_++; }
	virtual void windowDestroyed(Window& win) override { windowCount_--; }
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
}

void MyShellModule::render(Output& outp, ny::DrawContext& dc)
{
	if(!myTexture.glTexture())
	{
		ny::Image myImage;
		ny::sendLog("loading wallpaper... ", myImage.load("wallpaper.png"));

		myTexture.create(myImage);
	}
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	ny::TextureBrush texBrush;
	texBrush.extents.position = {0.f, 0.f};
	texBrush.extents.size = outp.size();
	texBrush.texture = &myTexture;

	ny::Rectangle bg = texBrush.extents;
	dc.mask(bg);
	dc.fill(texBrush);

	for(unsigned int i(0); i < windowCount_; ++i)
	{
		ny::Rectangle r({50 + 100.f * i, 50.f}, {80.f, 80.f});
		dc.mask(r);
		dc.fill(ny::Color(200, 150, 230));
	}

	ny::sendLog("Shell: Drawing ", outp.mappedSurfaces().size(), " mapped surfaces");
	SurfaceRes* cursorSurface = nullptr;
	for(auto& surf : outp.mappedSurfaces())
	{
		if(surf->roleType() == surfaceRoleType::cursor)
			cursorSurface = surf;

		ny::Rectangle surfaceRect(surf->extents());
		dc.mask(surfaceRect);

		ny::TextureBrush surfaceBrush;
		surfaceBrush.extents = surf->extents();
		surfaceBrush.texture = surf->surfaceContext()->content();

		dc.fill(surfaceBrush);

		surf->sendFrameDone();
	}

	if(cursorSurface)
	{
		SurfaceRes* surf = cursorSurface;
		ny::Rectangle surfaceRect(surf->extents());
		dc.mask(surfaceRect);

		ny::TextureBrush surfaceBrush;
		surfaceBrush.extents = surf->extents();
		surfaceBrush.texture = surf->surfaceContext()->content();

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
		cursorBrush.extents.position = seat_->pointer()->position();
		cursorBrush.extents.size = nytl::vec2f(30, 30);
		cursorBrush.texture = &cursorTexture;

		dc.mask(ny::Rectangle(cursorBrush.extents.position, nytl::vec2f(30, 30)));
		dc.fill(cursorBrush);
	}
}
