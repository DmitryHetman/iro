#pragma once

#include <iro/include.hpp>
#include <nyutil/nonCopyable.hpp>

#include <iro/compositor/resource.hpp>

class shell : public nonCopyable
{
protected:
    wl_global* global_ = nullptr;

    iroShellModule* module_ = nullptr;
    std::vector<iroShellExtension*> extensions_;

public:
    shell();
    ~shell();

    iroShellModule* getModule() const { return module_; }
    std::vector<iroShellExtension*> getExtensions() const { return extensions_; }

    void setModule(iroShellModule& mod);
    void addExtension(iroShellExtension& ext);
    void removeExtension(iroShellExtension& ext);
};

//////////////////////////////7
class shellRes : public resource
{
public:
    shellRes(wl_client& client, unsigned int id, unsigned int version);

    resourceType getType() const { return resourceType::shell; }
};
