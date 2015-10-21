#pragma once

#include <iro/include.hpp>
#include <iro/backend/output.hpp>
#include <ny/drawContext.hpp>

class iroDrawContext : public ny::drawContext
{
protected:
    ny::drawContext* dc_;
    output& output_;

    iroDrawContext(output& o) : ny::drawContext(o), output_(o) {};

public:
    virtual void drawSurface(output&, surfaceRes& surface) = 0; //additional

    output& getOutput() const { return output_; }

    //nyDC
    virtual void apply() override { dc_->apply(); }
	virtual void clear(ny::color col = ny::color::none) override { dc_->clear(col); }

	virtual void mask(const ny::mask& obj) override { dc_->mask(obj); }
	virtual void mask(const ny::path& obj) override { dc_->mask(obj); }
	virtual void resetMask() override { dc_->resetMask(); }

    virtual void mask(const ny::customPath& obj) override { dc_->mask(obj); }
	virtual void mask(const ny::text& obj) override { dc_->mask(obj); }

	virtual void mask(const ny::rectangle& obj) override { dc_->mask(obj); }
	virtual void mask(const ny::circle& obj) override { dc_->mask(obj); }

	virtual void fillPreserve(const ny::brush& col) override { dc_->fillPreserve(col); }
	virtual void strokePreserve(const ny::pen& col) override { dc_->strokePreserve(col); }

    virtual rect2f getClip() override { return dc_->getClip(); }
    virtual void clip(const rect2f& obj) override { dc_->clip(obj); }
	virtual void resetClip() override { dc_->resetClip(); }

	virtual void draw(const ny::shape& obj) override { dc_->draw(obj); }
};
