#include <iro/backend/backend.hpp>
#include <iro/backend/output.hpp>
#include <iro/backend/renderer.hpp>

unsigned char iroBackendType()
{
    if(!iroBackend())return backendType::none;
    return iroBackend()->getType();
}

