#include <iro/util/iroModule.hpp>
#include <iro/iro.hpp>

//module
bool iroModule::onLoad(moduleLoader& loader)
{
    iro* obj = dynamic_cast<iro*>(&loader);
    if(!obj)
        return 0;

    return onLoad(*obj);
}
