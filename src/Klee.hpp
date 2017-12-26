#include "rack.hpp"
using namespace rack;
extern Plugin *plugin;

#include "dsp/digital.hpp"

////////////////////
// module widgets
////////////////////

struct KleeWidget : ModuleWidget
{
	KleeWidget();
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
        if(rand() >= RAND_MAX/2)
            setValue(1.0);
        else
            setValue(0.0);
    }
};

struct NKK3 : NKK
{
    void randomize() override
    {
        float f = RAND_MAX;
        setValue((rand() / f) * 3.0);
    }
};

struct CKSS2 : CKSS
{
    void randomize() override
    {
        if(rand() >= RAND_MAX/2)
            setValue(1.0);
        else
            setValue(0.0);
    }
};
