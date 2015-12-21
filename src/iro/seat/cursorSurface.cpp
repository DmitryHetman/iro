#include <iro/seat/cursorSurface.hpp>
#include <iro/seat/pointer.hpp>

namespace iro
{

nytl::vec2i CursorSurfaceRole::position() const
{
	return pointer_->position() - hotspot_;
}

}
