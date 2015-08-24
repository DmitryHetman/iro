#include <iro/backend/renderer.hpp>
<<<<<<< HEAD
#include <iro/compositor/buffer.hpp>
=======
#include <iro/compositor/surface.hpp>
#include <iro/backend/backend.hpp>
>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527

/////////////////////////////////////////
renderer::renderer()
{
    iroBackend()->onOutputCreated(memberCallback(&renderer::initOutput, this));
    iroBackend()->onOutputDestroyed(memberCallback(&renderer::uninitOutput, this));
}

renderer::~renderer()
{
}

<<<<<<< HEAD
void renderer::setBufferSize(bufferRes& buff, vec2ui size) const
{
    buff.size_ = size;
}
void renderer::setBufferData(bufferRes& buff, bufferData* data) const
{
    buff.data_ = data;
}
void renderer::setBufferFormat(bufferRes& buff, bufferFormat format) const
{
    buff.format_ = format;
}
bufferData* renderer::getBufferData(bufferRes& buff) const
{
    return buff.data_;
}


=======
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

>>>>>>> 13bffabe7b15c8003eb9856e874841aad3236527
