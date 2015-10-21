#pragma once

#include <iro/include.hpp>
#include <iro/util/iroModule.hpp>

#include <vector>

//shellModule
class iroShellModule : public iroModule
{
public:
    virtual void render(iroDrawContext& dc, const std::vector<surfaceRes*>& mappedSurfaces) = 0;

    //iroModule
    virtual iroModuleType getType() const override final { return iroModuleType::shell; }
};

