#include <backend/backend.hpp>
#include <backend/output.hpp>

eglContext* getEglContext()
{
    if(!getBackend())return nullptr;
    return getBackend()->getEglContext();
}

backendType getBackendType()
{
    if(!getBackend())return backendType::none;
    return getBackend()->getType();
}


/////////////////////////////77
backend::~backend()
{
    for(unsigned int i(0); i < outputs_.size(); i++)
    {
        delete outputs_[i];
    }
}
