#pragma once
#define LVL_ON    (10.0)
#define LVL_OFF   (0.0)

using namespace rack;
extern Plugin *plugin;

struct PJ301YPort : SVGPort {
	PJ301YPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/PJ301Y.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct PJ301GPort : SVGPort {
	PJ301GPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/PJ301G.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct PJ301RPort : SVGPort {
	PJ301RPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/PJ301R.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct PJ301WPort : SVGPort {
	PJ301WPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/PJ301W.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct SchmittTrigger2
{
	// UNKNOWN is used to represent a stable state when the previous state is not yet set
	enum { UNKNOWN, LOW, HIGH } state = UNKNOWN;
	float low = 0.0;
	float high = 1.0;
	void setThresholds(float low, float high)
	{
		this->low = low;
		this->high = high;
	}
	/** Returns true if triggered */
	int process(float in) {
		switch (state) {
		case LOW:
			if (in >= high) {
				state = HIGH;
				return 1;
			}
			break;
		case HIGH:
			if (in <= low) {
				state = LOW;
				return -1;
			}
			break;
		default:
			if (in >= high) {
				state = HIGH;
			}
			else if (in <= low) {
				state = LOW;
			}
			break;
		}
		return 0;
	}

	void reset() {
		state = UNKNOWN;
	}
};

struct NKK2 : NKK
{
    void randomize() override
    {
        if(randomf() >= 0.5)
            setValue(1.0);
        else
            setValue(0.0);
    }
};

struct NKK3 : NKK
{
    void randomize() override
    {
        setValue(randomf() * maxValue);
    }
};


struct CKSS2 : CKSS
{
    void randomize() override
    {
        if(randomf() >= 0.5)
            setValue(1.0);
        else
            setValue(0.0);
    }
};

