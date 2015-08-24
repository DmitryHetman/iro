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
    std::cout << "iro-desktop-shell loaded" << std::endl;
    return 1;
}

void desktopShell::render()
{

}

void desktopShell::renderSurface(surfaceRes& surf)
{

}

