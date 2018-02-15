#include "common.hpp"
#include "dsp/digital.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

#ifdef TEST_MODULE
////////////////////
// module widgets
////////////////////
using namespace rack;
extern Plugin *plugin;

struct LaunchpadTestWidget : ModuleWidget
{
	LaunchpadTestWidget();
};

struct PatternBtn : SVGSwitch, ToggleSwitch {
	PatternBtn() {
		addFrame(SVG::load(assetPlugin(plugin, "res/Patternbtn_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Patternbtn_1.svg")));
	}
};
#endif
