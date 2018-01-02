#include "rack.hpp"
#include "common.hpp"

#include <algorithm>
#include "dsp/digital.hpp"

struct KleeWidget : ModuleWidget
{
private:
    enum MENUACTIONS
    {
        RANDOMIZE_BUS,
        RANDOMIZE_PITCH,
        RANDOMIZE_LOAD,
        SET_RANGE_1V
    };

    struct KleeMenuItem : MenuItem
    {
        KleeMenuItem(const char *title, KleeWidget *pW, MENUACTIONS act)
        {
            text = title;
            widget = pW;
            action = act;
        };

        void onAction(EventAction &e) override {widget->onMenu(action);};

        private:
            KleeWidget *widget;
            MENUACTIONS action;
    };

    int getParamIndex(int index)
    {
        auto it = std::find_if(params.begin(), params.end(), [&index](const ParamWidget *m) -> bool { return m->paramId == index; });
        if(it != params.end())
            return std::distance(params.begin(), it);

        return -1;
    }

    void std_randomize(int first_index)
    {
        for (int k = 0; k < 16; k++)
        {
            int index = getParamIndex(first_index + k);
            if(index >= 0)
                params[index]->randomize();
        }
    }

    public:
        KleeWidget();
        Menu *createContextMenu() override;
        void onMenu(MENUACTIONS action);
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

