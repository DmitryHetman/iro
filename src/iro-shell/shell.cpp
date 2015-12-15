#include <iro/compositor/shell.hpp>
#include <iro/compositor/surface.hpp>
#include <iro/backend/surfaceContext.hpp>
#include <iro/backend/output.hpp>
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
	Shell* myShell_ = nullptr;
	ny::GlTexture myTexture;

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
	if(myShell_) delete myShell_;
}

void MyShellModule::init(Compositor& comp, Seat& seat)
{
	compositor_ = &comp;
	seat_ = &seat;

	myShell_ = new Shell(comp);
}

void MyShellModule::render(Output& outp, ny::DrawContext& dc)
{
	nytl::sendLog("Drawing shell...");

	if(!myTexture.glTexture())
	{
		ny::Image myImage;
		nytl::sendLog("loading wallpaper... ", myImage.load("wallpaper.png"));

		myTexture.create(myImage);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	ny::TextureBrush texBrush;
	texBrush.extents.position = {0.f, 0.f};
	texBrush.extents.size = outp.size();
	texBrush.texture = &myTexture;

	nytl::sendLog(":1");
	dc.clear(texBrush);
	nytl::sendLog(":2");

	nytl::sendLog("Drawing ", outp.mappedSurfaces().size(), " mapped surfaces");
	for(auto& surf : outp.mappedSurfaces())
	{
		ny::Rectangle surfaceRect(surf->extents());
		dc.mask(surfaceRect);

		ny::TextureBrush surfaceBrush;
		surfaceBrush.extents = surf->extents();
		surfaceBrush.texture = surf->surfaceContext()->content();

		dc.fill(surfaceBrush);
	}
}
