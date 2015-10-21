#include <iro/backend/renderer.hpp>
#include <iro/compositor/surface.hpp>
#include <iro/backend/backend.hpp>
#include <iro/backend/output.hpp>

/////////////////////////////////////////
renderer::renderer()
{
    iroBackend()->onOutputCreated(memberCallback(&renderer::initOutput, this));
    iroBackend()->onOutputDestroyed(memberCallback(&renderer::uninitOutput, this));
}

renderer::~renderer()
{
}

void renderer::setOutputData(output& o, renderData* data) const
{
    o.renderData_ = data;
}
void renderer::setSurfaceData(surfaceRes& surf, renderData* data) const
{
    surf.renderData_ = data;
}

renderData* renderer::getOutputData(output& o) const
{
    return o.renderData_;
}
renderData* renderer::getSurfaceData(surfaceRes& surf) const
{
    return surf.renderData_;
}

