#include <iro/compositor/shell.hpp>
#include <iro/compositor/surface.hpp>
#include <iro/compositor/xdgShell.hpp>
#include <iro/backend/surfaceContext.hpp>
#include <iro/backend/output.hpp>
#include <iro/seat/seat.hpp>
#include <iro/seat/pointer.hpp>
#include <ny/draw/drawContext.hpp>
#include <ny/draw/image.hpp>
#include <ny/draw/gl/glTexture.hpp>
using namespace iro;

#include <glpbinding/glp20/glp.h>
using namespace glp20;

#include <nytl/log.hpp>

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

public:
	virtual ~MyShellModule();

	virtual void init(Compositor& comp, Seat& s) override;
	virtual void render(Output& outp, ny::DrawContext& dc) override;
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
		nytl::sendLog("loading wallpaper... ", myImage.load("wallpaper.png"));

		myTexture.create(myImage);
	}
	
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	ny::TextureBrush texBrush;
	texBrush.extents.position = {0.f, 0.f};
	texBrush.extents.size = outp.size();
	texBrush.texture = &myTexture;

	dc.clear(texBrush);

	nytl::sendLog("Shell: Drawing ", outp.mappedSurfaces().size(), " mapped surfaces");
	bool renderedCursor_ = 0;
	for(auto& surf : outp.mappedSurfaces())
	{
		nytl::sendLog("surf position: ", surf->position());

		ny::Rectangle surfaceRect(surf->extents());
		dc.mask(surfaceRect);

		ny::TextureBrush surfaceBrush;
		surfaceBrush.extents = surf->extents();
		surfaceBrush.texture = surf->surfaceContext()->content();

		dc.fill(surfaceBrush);

		if(surf->roleType() == surfaceRoleType::cursor)
			renderedCursor_ = 1;

		surf->sendFrameDone();
	}

	if(!renderedCursor_)
	{
		if(!cursorTexture.glTexture())
		{
			ny::Image cursorImage;
			nytl::sendLog("loading cursor image: ", cursorImage.load("cursor.png"));
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
