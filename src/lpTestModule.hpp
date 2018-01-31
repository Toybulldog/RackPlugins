#include "common.hpp"

#include "dsp/digital.hpp"


#include <sstream>
#include <iomanip>

#include <algorithm>

#if defined(ARCH_WIN) && defined(TEST_MODULE)
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
		addFrame(SVG::load(assetPlugin(plugin,"res/Patternbtn_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/Patternbtn_1.svg")));
	}
};
#endif
