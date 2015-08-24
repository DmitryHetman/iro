#pragma once

#include <iro/include.hpp>
#include <nyutil/module.hpp>

//type
enum class iroModuleType
{
    shell,
    shellExtension,
    custom
};

//module
class iroModule : public module
{
public:
    virtual bool onLoad(moduleLoader& loader) override final;

    virtual bool onLoad(iro& obj) = 0;
    virtual iroModuleType getType() const = 0;
};
//shellExtension
class iroShellExtension : public iroModule
{

};
