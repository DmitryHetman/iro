#pragma once

#include <iro/include.hpp>
#include <nyutil/nonCopyable.hpp>

//type
enum class iroModuleType
{
    shell,
    shellExtension,
    custom
};

//module
class iroModule : public nonCopyable
{
protected:
    static iroModule* global_;

public:
    iroModule(){ global_ = this; }
    ~iroModule(){ global_ = nullptr; };

    virtual bool onLoad(iro& obj) = 0;
    virtual iroModuleType getType() const = 0;

    static iroModule* getGlobal() { return global_; }
};

//shellExtension
class iroShellExtension : public iroModule
{

};
