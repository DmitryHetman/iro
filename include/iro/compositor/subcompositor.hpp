#pragma once

#include <iro/include.hpp>
#include <iro/util/global.hpp>

namespace iro
{

///The Subcompositor class is resposbile for managing subsurfaces.
class Subcompositor : public Global
{
protected:
	Compositor* compositor_;

public:
    Subcompositor(Compositor& comp);
    ~Subcompositor();

	Compositor& compositor() const { return *compositor_; }
};


}
