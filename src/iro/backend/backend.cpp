#include <iro/backend/backend.hpp>
#include <iro/backend/output.hpp>
#include <iro/backend/renderer.hpp>

eglContext* iroEglContext()
{
    if(!iroBackend())return nullptr;
    return iroBackend()->getEglContext();
}

backendType iroBackendType()
{
    if(!iroBackend())return backendType::none;
    return iroBackend()->getType();
}

///////////////////////////////////////////////////////

backend::backend()
{
}

backend::~backend()
{
    if(renderer_) delete renderer_;
}
