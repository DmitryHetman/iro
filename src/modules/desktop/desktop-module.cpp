#include <iro/util/iroModule.hpp>
#include <iostream>

class desktopShell : public iroShellModule
{
public:
    virtual bool onLoad(iro& obj) override;

    virtual void render() override;
    virtual void renderSurface(surfaceRes& surf) override;
};

desktopShell module;

//implementation
bool desktopShell::onLoad(iro& obj)
{
    return 1;
}

void desktopShell::render()
{

}

void desktopShell::renderSurface(surfaceRes& surf)
{

}

