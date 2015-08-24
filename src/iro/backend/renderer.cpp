#include <iro/backend/renderer.hpp>
#include <iro/compositor/buffer.hpp>

/////////////////////////////////////////
renderer::renderer()
{
}

renderer::~renderer()
{
}

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


