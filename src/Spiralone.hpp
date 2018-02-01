#include "common.hpp"

#include "dsp/digital.hpp"


#include <sstream>
#include <iomanip>

#include <algorithm>

////////////////////
// module widgets
////////////////////


struct SpiraloneWidget : ModuleWidget
{
public:
   	SpiraloneWidget();

private:
    void createSequencer(int seq);
};


