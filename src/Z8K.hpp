#include "rack.hpp"
#include "common.hpp"

#include "dsp/digital.hpp"


#include <sstream>
#include <iomanip>

#include <algorithm>

////////////////////
// module widgets
////////////////////

#define COLOR_HALF_YELLOW nvgRGB(0x7c, 0x6f, 0xe)
#define COLOR_HALF_ORANGE nvgRGB(0x79, 0x58, 0x10)
#define COLOR_HALF_PURPLE nvgRGB(0x6A, 0x15, 0x76)
#define COLOR_HALF_CYAN nvgRGB(0x11, 0x73, 0x77)
#define COLOR_HALF_RED nvgRGB(0x76, 0x16, 0x12)
#define COLOR_HALF_WHITE nvgRGB(0x7f, 0x7f, 0x7f)
#define COLOR_HALF_GREEN nvgRGB(0x48, 0x63, 0x1F)

struct MultiColorLight : ModuleLightWidget {
	MultiColorLight() {// led off. bit 0: row sequencer (1-4); bit 1: column sequencer (A-D); bit 2: horizontal sequencer; bit 3: vertical sequencer
		addBaseColor(COLOR_RED);	// bit 0: row sequencer (1-4); V = 1
		addBaseColor(COLOR_YELLOW); // bit 1: column sequencer (A-D); V = 2
		addBaseColor(COLOR_ORANGE); // bit 0+1: V = 3
		addBaseColor(COLOR_GREEN);  // bit 2: horizontal sequencer; V = 4
		addBaseColor(COLOR_HALF_ORANGE);  // bit 2+0: horizontal sequencer; V = 5
		addBaseColor(COLOR_HALF_YELLOW);  // bit 2+1: horizontal sequencer; V = 6
		addBaseColor(COLOR_CYAN);  // bit 2+1+0: horizontal sequencer; V = 7
		addBaseColor(COLOR_BLUE);  // bit 3: vertical sequencer; V = 8		
		addBaseColor(COLOR_HALF_RED);  // bit 3+0: vertical sequencer; V = 9
		addBaseColor(COLOR_HALF_PURPLE);  // bit 3+1: vertical sequencer; V = 10		
		addBaseColor(COLOR_HALF_CYAN);  // bit 3+1+0: vertical sequencer; V = 11
		addBaseColor(COLOR_PURPLE);  // bit 3+2: vertical sequencer; V = 12
		addBaseColor(COLOR_HALF_GREEN);  // bit 3+2+0: vertical sequencer; V = 13
		addBaseColor(COLOR_HALF_WHITE);  // bit 3+2+1: vertical sequencer; V = 14
		addBaseColor(COLOR_WHITE);  // bit 3+2+1+0: vertical sequencer; V = 15
	}
};

struct Z8KWidget : ModuleWidget
{
public:
   	Z8KWidget();
};


