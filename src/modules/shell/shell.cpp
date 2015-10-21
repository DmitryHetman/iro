#include <iro/compositor/shell.hpp>
#include <iro/backend/egl.hpp>
#include <iro/seat/pointer.hpp>
#include <iro/util/iroDrawContext.hpp>
#include <iro/util/log.hpp>

#include <ny/shape.hpp>

#include <iostream>

class waylandShell;
class xdgShell;

class desktopShell : public iroShellModule
{
protected:
    waylandShell* wlShell_ = nullptr;
    xdgShell* xdgShell_ = nullptr;

public:
    virtual bool onLoad(iro& obj) override;
    virtual void render(iroDrawContext& dc, const std::vector<surfaceRes*>& mappedSurfaces) override;
};

desktopShell module;

//implementation
bool desktopShell::onLoad(iro& obj)
{
    std::cout << "iro-desktop-shell loaded" << std::endl;
    return 1;
}

void desktopShell::render(iroDrawContext& dc, const std::vector<surfaceRes*>& mappedSurfaces)
{
    iroLog("iro-desktop-shell rendering: ", iroEglContext()->isCurrent());

    dc.clear(ny::color(200, 240, 231));

    for(auto surf : mappedSurfaces)
        if(surf) dc.drawSurface(dc.getOutput(), *surf);

    dc.mask(ny::rectangle(iroPointer()->getPosition(), vec2i(50, 50)));
    dc.fill(ny::color::black);
}


