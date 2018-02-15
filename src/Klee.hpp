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

		void onAction(EventAction &e) override { widget->onMenu(action); };

	private:
		KleeWidget * widget;
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
		for(int k = 0; k < 16; k++)
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



