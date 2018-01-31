#include "common.hpp"
#include "lpTestModule.hpp"
#include <string.h>
#include "dsp/digital.hpp"

#ifdef TEST_MODULE

struct LaunchpadTest : Module
{
	enum ParamIds {
	    BTN1,
	    BTN2,
	    BTN3,
	    BTN4,
	    GATE_TIME,
	    NUM_PARAMS
	};
	enum InputIds {
		FROM_LP,
		NUM_INPUTS
	};
	enum OutputIds {
		TO_LP,
		NUM_OUTPUTS
	};
	enum LightIds {
		LP_CONNECTED,
		NUM_LIGHTS
	};
	LaunchpadTest() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
        drv = new LaunchpadBindingDriver(Scene1, 1);
    }
    ~LaunchpadTest()
    {
        delete drv;
    }
	void step() override;

    LaunchpadBindingDriver *drv;
};

void LaunchpadTest::step()
{
    lights[LP_CONNECTED].value = params[LaunchpadTest::BTN2].value > 0 ? 1.0 : 0.0;

    drv->ProcessLaunchpad();
}

LaunchpadTestWidget::LaunchpadTestWidget()
{
	LaunchpadTest *module = new LaunchpadTest();
	setModule(module);
	box.size = Vec(13 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		LightPanel *panel = new LightPanel();
		panel->box.size = box.size;
		addChild(panel);
	}

	addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

	ParamWidget *pctrl = createParam<PatternBtn>(Vec(20, 20), module, LaunchpadTest::BTN1, 0.0, 1.0, 0.0);
	LaunchpadLed offColor;
	LaunchpadLed onColor;
	offColor.r_color=20;
	onColor.r_color = 3;
	LaunchpadSwitch *sw1 = new LaunchpadSwitch(0, LaunchpadKey::R1C1, offColor, onColor);
	module->drv->Add(sw1, pctrl);
	addParam(pctrl);

	ModuleLightWidget *plight = createLight<MediumLight<GreenLight>>(Vec(50, 20), module, LaunchpadTest::LP_CONNECTED);
	onColor.r_color = 9;
	LaunchpadLight *ld1 = new LaunchpadLight(0, LaunchpadKey::R8C1, offColor, onColor);
	module->drv->Add(ld1, plight);
	addChild(plight);

  LaunchpadRadio *radio = new LaunchpadRadio(0, ILaunchpadPro::RC2Key(5,2), 3, LaunchpadLed::Color(2), LaunchpadLed::Color(3));

    ParamWidget *pEna = createParam<CKSSThree>(Vec(36 + 80, RACK_GRID_HEIGHT-58), module, LaunchpadTest::BTN2, 0.0, 2.0, 1.0);
    module->drv->Add(radio, pEna);
    radio = new LaunchpadRadio(0, ILaunchpadPro::RC2Key(3,0), 3, LaunchpadLed::Color(2), LaunchpadLed::Color(3), true);
	addParam(pEna);

   pEna = createParam<CKSSThree>(Vec(36 + 50, RACK_GRID_HEIGHT-58), module, LaunchpadTest::BTN3, 0.0, 2.0, 1.0);
    module->drv->Add(radio, pEna);
addParam(pEna);

    LaunchpadMomentary *mome = new LaunchpadMomentary(0, ILaunchpadPro::RC2Key(1,7), LaunchpadLed::Color(2), LaunchpadLed::Color(3));
    pEna = createParam<BefacoPush>(Vec(96, 20), module, LaunchpadTest::BTN4, 0.0, 1.0, 0.0);
    module->drv->Add(mome, pEna);
addParam(pEna);

    LaunchpadKnob *pknob = new LaunchpadKnob(0, ILaunchpadPro::RC2Key(6,6), LaunchpadLed::Rgb(20,10,10), LaunchpadLed::Rgb(60,40,40));
    pEna = createParam<Davies1900hBlackKnob>(Vec(50, 100), module, LaunchpadTest::GATE_TIME, 0.005, 1.0, 0.25);
    module->drv->Add(pknob, pEna);
addParam(pEna);
#ifdef DEBUG
info("RDY");
#endif
}
#endif
