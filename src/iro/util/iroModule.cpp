#include <iro/util/iroModule.hpp>
#include <iro/iro.hpp>

iroModule* iroModule::global_ = nullptr;

extern "C" iroModule* iro_moduleLoadFunc(iro& obj)
{
    if(iroModule::getGlobal())
    {
        if(iroModule::getGlobal()->onLoad(obj))
            return iroModule::getGlobal();
    }


    return nullptr;
}
