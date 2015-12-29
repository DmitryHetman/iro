#pragma once

#include <iro/include.hpp>
#include <stdexcept>

#include <wayland-server-core.h>

namespace iro
{

template <typename T>
T& wlArrayPush(wl_array& array, const T& value)
{
	T* data = static_cast<T*>(wl_array_add(&array, sizeof(value)));
	if(data)
	{
		*data = value;
		return *data;
	}
	else
	{
		throw std::bad_alloc();
	}
}

}
