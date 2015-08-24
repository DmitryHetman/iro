#pragma once

#include <iro/util/iroModule.hpp>

//shellModule
class iroShellModule : public iroModule
{
public:
    virtual void render(iroDrawContext& dc, std::vector<surfaceRes*> mappedSurfaces) = 0;

    //iroModule
    virtual iroModuleType getType() const override final { return iroModuleType::shell; }
};

