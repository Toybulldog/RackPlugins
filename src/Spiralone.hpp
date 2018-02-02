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


struct SpiraloneWidget : ModuleWidget
{
public:
   	SpiraloneWidget();

private:
    void createSequencer(int seq);
    ModuleLightWidget *createLed(int seq, Vec pos, Module *module,int firstLightId, bool big = false);
    NVGcolor  color[NUM_SEQUENCERS];


};


