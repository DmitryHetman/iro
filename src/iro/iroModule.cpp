#include <iro/iroModule.hpp>
#include <iro/iro.hpp>

//loader
iroModule* iroModuleLoader::loadModule(const std::string& modName)
{
    module* mod = moduleLoader::loadModule(modName);
    if(mod)
    {
        iroModule* ret = dynamic_cast<iroModule*>(mod);
        if(!ret)
        {
            moduleLoader::unloadModule(*mod);
            return nullptr;
        }
        return ret;
    }

    return nullptr;
}

//module
bool iroModule::onLoad(moduleLoader& loader)
{
    iro* obj = dynamic_cast<iro*>(&loader);
    if(!obj)
        return 0;

    return onLoad(*obj);
}
