#pragma once

#include <iro.hpp>
#include <util/nonCopyable.hpp>
#include <shader/shader.hpp>

class renderer : public nonCopyable
{
public:
    renderer();
    virtual ~renderer();

    bool render(surfaceRes* surface);
};
