#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>

#include <nytl/nonCopyable.hpp>
#include <nytl/vec.hpp>
#include <nytl/callback.hpp>
#include <nytl/watchable.hpp>

namespace iro
{

///Represents a physical pointer.
class Pointer : public nytl::nonCopyable
{
protected:
    Seat* seat_;

    SurfaceRef over_;
	nytl::vec2i position_;

    //if this is == nullptr, the default cursor will be used
    bool customCursor_ = 0; //cursor is used
    SurfaceRef cursor_;
	nytl::vec2i hotspot_;

	nytl::callback<void(const nytl::vec2i& pos)> moveCallback_;
	nytl::callback<void(unsigned int button, bool press)> buttonCallback_;
	nytl::callback<void(unsigned int axis, double value)> axisCallback_;
	nytl::callback<void(SurfaceRes*, SurfaceRes*)> focusCallback_;

    //helper func for sendMove
    void setOver(SurfaceRes* newOne);

public:
	Pointer(Seat& seat);
	~Pointer();

    void sendMove(const nytl::vec2i& pos);
    void sendButton(unsigned int button, bool press);
    void sendAxis(unsigned int axis, double value);

    SurfaceRes* overSurface() const { return over_.get(); }
    PointerRes* activeResource() const;

    Seat& seat() const { return *seat_; }
	Compositor& compositor() const;

    void cursor(SurfaceRes& surf, const nytl::vec2i& hotspot);
    void resetCursor();
    SurfaceRes* cursor() const { return cursor_.get(); }

	nytl::vec2i position() const { return position_; }
	nytl::vec2i wlFixedPosition() const;

    //callbacks
	template<typename F> nytl::connection onMove(F&& f)
		{ return moveCallback_.add(f); }
    template<typename F> nytl::connection onButton(F&& f)
		{ return buttonCallback_.add(f); }
    template<typename F> nytl::connection onAxis(F&& f)
		{ return axisCallback_.add(f); }
    template<typename F> nytl::connection onFocusChange(F&& f)
		{ return focusCallback_.add(f); }
};

///Represents a clients pointer resource.
class PointerRes : public Resource
{
protected:
    SeatRes* seatRes_;

public:
	PointerRes(SeatRes& seatRes, unsigned int id);
	~PointerRes() = default;

    SeatRes& seatRes() const { return *seatRes_; }
    Seat& seat() const;
    Pointer& pointer() const;

    virtual unsigned int type() const override { return resourceType::pointer; }
};

}
