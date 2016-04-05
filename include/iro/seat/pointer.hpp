#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>

#include <nytl/nonCopyable.hpp>
#include <nytl/vec.hpp>
#include <nytl/callback.hpp>
#include <nytl/observe.hpp>

#include <functional>

namespace iro
{

///Represents a physical pointer. 
class Pointer : public nytl::NonCopyable
{
public:
	struct Grab
	{
		bool exclusive;
		std::function<void(const nytl::Vec2i&, const nytl::Vec2i&)> moveFunction;
		std::function<void(unsigned int button, bool press)> buttonFunction;
		std::function<void(unsigned int axis, double value)> axisFunction;
		std::function<void(bool)> grabEndFunction; //if parameter==1, there is a new grab
	};

protected:
    Seat* seat_;

    SurfacePtr over_;
	nytl::Vec2i position_;

	bool grabbed_ = 0;
	Grab grab_;

    bool customCursor_ = 0; //cursor is used
    SurfacePtr cursor_;
	nytl::Vec2i hotspot_;

	nytl::Callback<void(const nytl::Vec2i& pos, const nytl::Vec2i& delta)> moveCallback_;
	nytl::Callback<void(unsigned int button, bool press)> buttonCallback_;
	nytl::Callback<void(unsigned int axis, double value)> axisCallback_;
	nytl::Callback<void(SurfaceRes*, SurfaceRes*)> focusCallback_;

    //helper func for sendMove
    void setOver(SurfaceRes* newOne);

public:
	Pointer(Seat& seat);
	~Pointer();

    void sendMove(const nytl::Vec2i& pos);
    void sendButton(unsigned int button, bool press);
    void sendAxis(unsigned int axis, double value);

    SurfaceRes* overSurface() const { return over_.get(); }
    PointerRes* activeResource() const;

    Seat& seat() const { return *seat_; }
	Compositor& compositor() const;

    void cursor(SurfaceRes& surf, const nytl::Vec2i& hotspot);
    void resetCursor();
    SurfaceRes* cursor() const { return cursor_.get(); }

	nytl::Vec2i position() const { return position_; }
	nytl::Vec2i wlFixedPosition() const;
	nytl::Vec2i wlFixedPositionRelative() const;

	bool grab(const Grab& grab, bool force = 1);
	bool releaseGrab();

    //callbacks
	template<typename F> nytl::Connection onMove(F&& f)
		{ return moveCallback_.add(f); }
    template<typename F> nytl::Connection onButton(F&& f)
		{ return buttonCallback_.add(f); }
    template<typename F> nytl::Connection onAxis(F&& f)
		{ return axisCallback_.add(f); }
    template<typename F> nytl::Connection onFocusChange(F&& f)
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
