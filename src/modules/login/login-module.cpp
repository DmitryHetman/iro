#include <iro/util/iroModule.hpp>
#include <iostream>

class loginShell : public iroShellModule
{
public:
    virtual bool onLoad(iro& obj) override;

    virtual void render() override;
    virtual void renderSurface(surfaceRes& surf) override;
};

loginShell module;

//implementation
bool loginShell::onLoad(iro& obj)
{
    std::cout << "iro-login-shell loaded" << std::endl;
    return 1;
}

void loginShell::render()
{

}

void loginShell::renderSurface(surfaceRes& surf)
{

}

