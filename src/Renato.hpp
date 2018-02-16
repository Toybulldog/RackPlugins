#include "common.hpp"
#include "dsp/digital.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

////////////////////
// module widgets
////////////////////


struct RenatoWidget : SequencerWidget
{	
public:
	void onMenu(int action);
	RenatoWidget();

private:
	enum MENUACTIONS
	{
		RANDOMIZE_PITCH,
		RANDOMIZE_GATEX,
		RANDOMIZE_GATEY,
		RANDOMIZE_ACCESS
	};

	Menu *addContextMenu(Menu *menu) override;
};


