#include "common.hpp"

#include "dsp/digital.hpp"


#include <sstream>
#include <iomanip>

#include <algorithm>

////////////////////
// module widgets
////////////////////


struct RenatoWidget : ModuleWidget
{
    enum MENUACTIONS
    {
        RANDOMIZE_PITCH,
        RANDOMIZE_GATEX,
        RANDOMIZE_GATEY,
        RANDOMIZE_ACCESS
    };

public:
   	RenatoWidget();
    Menu *createContextMenu() override;
    void onMenu(MENUACTIONS action);

struct KleeMenuItem : MenuItem
    {
        KleeMenuItem(const char *title, RenatoWidget *pW, MENUACTIONS act)
        {
            text = title;
            widget = pW;
            action = act;
        };

        void onAction(EventAction &e) override {widget->onMenu(action);};

        private:
            RenatoWidget *widget;
            MENUACTIONS action;
    };
private:
    MENUACTIONS action;

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

};


