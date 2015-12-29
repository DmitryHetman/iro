#pragma once

#include <iro/include.hpp>
#include <iro/compositor/window.hpp>

namespace iro
{

///Represents an client window from the xwayland server
class XWindow : public Window
{
protected:
	unsigned int id_;

public:
	XWindow(unsigned int id);
	virtual void close() override;
};

}
