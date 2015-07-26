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

//shellModule
class iroShellModule : public iroModule
{
public:
    virtual iroModuleType getType() const override final { return iroModuleType::shell; }

    virtual void render() = 0;
    virtual void renderSurface(surfaceRes& surf) = 0;
};

//shellExtension
class iroShellExtension : public iroModule
{

};

//iroModuleLoader
class iroModuleLoader : public moduleLoader
{
public:
    virtual iroModule* loadModule(const std::string& modName) override; //loads every iroModule
};
