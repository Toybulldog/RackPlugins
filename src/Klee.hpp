#include "common.hpp"
#include <algorithm>
#include "dsp/digital.hpp"

struct KleeWidget : SequencerWidget
{
private:
	enum MENUACTIONS
	{
		RANDOMIZE_BUS,
		RANDOMIZE_PITCH,
		RANDOMIZE_LOAD,
		SET_RANGE_1V
	};
	Menu *addContextMenu(Menu *menu) override;

public:
	KleeWidget();
	void onMenu(int action);
};



