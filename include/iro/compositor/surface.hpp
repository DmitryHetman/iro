#pragma once

#include <iro/include.hpp>
#include <iro/compositor/resource.hpp>
#include <iro/compositor/buffer.hpp>
#include <iro/compositor/callback.hpp>

#include <nyutil/vec.hpp>
#include <nyutil/region.hpp>
#include <nyutil/rect.hpp>

#include <vector>

//state////////////////////////////////////////////////////
class surfaceState
{
public:
    region opaque = region();
    region input = region();
    region damage = region();
    vec2i offset = vec2i();

    bufferRef buffer;
    std::vector<callbackRef> frameCallbacks;

    int scale = 1;
    unsigned int transform = 0;
    int zOrder = 0;

    void reset()
    {
        opaque = region();
        input = region();
        damage = region();
        offset = vec2i();
        buffer.set(nullptr);
        frameCallbacks.clear(); //todo ?
        scale = 1;
        transform = 0;
        zOrder = 0;
    }

    surfaceState& operator=(const surfaceState& other);
};

//roleType
namespace surfaceRoleType
{
    const unsigned char none = 0;
    const unsigned char shell = 1;
    const unsigned char sub = 2;
    const unsigned char cursor = 3;
    const unsigned char dataSource = 4;
}

class surfaceRole
{
public:
    virtual unsigned char getType() const = 0;
    virtual vec2i getPosition() const = 0;
    virtual bool isMapped() const = 0;
    virtual void commit() = 0;
};

//class///////////////////////////////////////////////////
class surfaceRes : public resource
{

friend renderer;

protected:
    surfaceState commited_;
    surfaceState pending_;

    unsigned int roleType_ = surfaceRoleType::none;
    surfaceRole* role_;

    std::vector<output*> outputs_; //all outputs this surface is mapped on

    //for renderer
    renderData* renderData_ = nullptr; //cache data from the renderer
    void frameDone();

public:
    surfaceRes(wl_client& client, unsigned int id);
    ~surfaceRes();

    void addFrameCallback(callbackRes& cb);
    void setInputRegion(region input);
	void setOpaqueRegion(region output);
 	void setBufferScale(int scale);
	void setBufferTransform(unsigned int transform);
    void attach(bufferRes& buff, vec2i pos);
	void damage(rect2i dmg);
    void commit();

    region getInputRegion() const { return commited_.input; }
    region getOpaqueRegion() const { return commited_.opaque; }
    int getBufferScale() const { return commited_.scale; }
    unsigned int getBufferTransform() const { return commited_.transform; }
    bufferRes* getAttachedBuffer() const { return commited_.buffer.get(); }
    region getDamage() const { return commited_.damage; }

    bool isMapped() const;

    surfaceRole* getRole() const { return role_; }
    unsigned char getRoleType() const { return roleType_; }

    vec2i getPosition() const;
    vec2ui getSize() const;
    rect2i getExtents() const;

    //resource
    resourceType getType() const { return resourceType::surface; }
};
