#include "common.hpp"

#include "dsp/digital.hpp"


#include <sstream>
#include <iomanip>

#include <algorithm>

#define NUM_SEQUENCERS (5)
#define TOTAL_STEPS (32)

////////////////////
// module widgets
////////////////////


struct SpiraloneWidget : SequencerWidget
{
public:
	SpiraloneWidget();
	Menu *addContextMenu(Menu *menu) override;
	void onMenu(int action);

private:
	enum MENUACTIONS
	{
		RANDOMIZE_PITCH,
		RANDOMIZE_LEN,
		RANDOMIZE_STRIDE,
		RANDOMIZE_XPOSE
	};
	void createSequencer(int seq);
	ModuleLightWidget *createLed(int seq, Vec pos, Module *module, int firstLightId, bool big = false);
	NVGcolor color[NUM_SEQUENCERS];
};


