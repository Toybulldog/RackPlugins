#include "Renato.hpp"
#include "rntSequencer.hpp"
#include <sstream>

struct Renato : Module
{
    enum ParamIds
    {
        COUNTMODE_X, COUNTMODE_Y,
        SEEKSLEEP,
        ACCESS_1,
        GATEX_1 = ACCESS_1+16,
        GATEY_1 = GATEX_1+16,
        VOLTAGE_1 = GATEY_1+16,
        NUM_PARAMS = VOLTAGE_1+16
    };

    enum InputIds
    {
        XCLK,
        YCLK,
        NUM_INPUTS
    };

    enum OutputIds
    {
        CV,
		XGATE, YGATE,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        LED_GATEX, LED_GATEY,
        LED_1,
        NUM_LIGHTS = LED_1+16
    };

    Renato() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
    {
        #ifdef LAUNCHPAD
        drv = new LaunchpadBindingDriver(Scene3, 2);
        drv->SetAutoPageKey(LaunchpadKey::SESSION, 0);
        drv->SetAutoPageKey(LaunchpadKey::NOTE, 1);
        #endif
        on_loaded();
    }

    #ifdef LAUNCHPAD
    ~Renato()
    {
        delete drv;
    }
    #endif

    void step() override;
    void reset() override {load();}

    void fromJson(json_t *root) override {Module::fromJson(root); on_loaded();}
    json_t *toJson() override
    {
		json_t *rootJ = json_object();
		return rootJ;
	}
    bool _accessX(int p) {return _access(xy(p, seqY.Position()));}
    bool _accessY(int p) {return _access(xy(seqX.Position(), p));}

	#ifdef LAUNCHPAD
    LaunchpadBindingDriver *drv;
    float connected;
    #endif

private:
    void on_loaded();
    void load();
    void led(int n) {for(int k = 0; k < 16; k++) lights[LED_1+k].value = k == n ? 10.0 : 0.0;}
    int xy(int x, int y) {return 4*y+x;}
    bool _access(int n)  {return params[ACCESS_1+n].value > 0; }
	bool _gateX(int n) {return params[GATEX_1+n].value > 0; }
	bool _gateY(int n) {return params[GATEY_1+n].value > 0; }
    rntSequencer seqX;
    rntSequencer seqY;
};

bool Access(Renato *pr, bool is_x, int p) {return is_x ? pr->_accessX(p) : pr->_accessY(p);}

void Renato::on_loaded()
{
    #ifdef LAUNCHPAD
    connected=0;
    #endif
    load();
}

void Renato::load()
{
    seqX.Reset();
    seqY.Reset();
}

void Renato::step()
{
    bool seek_mode = params[SEEKSLEEP].value > 0;
    int clkX = seqX.Step(inputs[XCLK].value, params[COUNTMODE_X].value, seek_mode, this, true);
    int clkY = seqY.Step(inputs[YCLK].value, params[COUNTMODE_Y].value, seek_mode, this, false);
    int n = xy(seqX.Position(), seqY.Position());
    if(_access(n))
    {
        if(_gateX(n))
            seqX.Gate(clkX, &outputs[XGATE], &lights[LED_GATEX]);

        if(_gateY(n))
            seqY.Gate(clkY, &outputs[YGATE], &lights[LED_GATEY]);

        outputs[CV].value = params[VOLTAGE_1+n].value;
        led(n);
    }

    #ifdef LAUNCHPAD
    connected = drv->Connected() ? 1.0 : 0.0;
    drv->ProcessLaunchpad();
    #endif
}

Menu *RenatoWidget::createContextMenu()
{
	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	menu->addChild(new KleeMenuItem("Randomize Pitch", this, RANDOMIZE_PITCH));
	menu->addChild(new KleeMenuItem("Randomize Gate Xs", this, RANDOMIZE_GATEX));
	menu->addChild(new KleeMenuItem("Randomize Gate Ys", this, RANDOMIZE_GATEY));
	menu->addChild(new KleeMenuItem("Randomize Access", this, RANDOMIZE_ACCESS));
	return menu;
}

void RenatoWidget::onMenu(MENUACTIONS action)
{
    switch(action)
    {
        case RANDOMIZE_PITCH: std_randomize(Renato::VOLTAGE_1); break;
        case RANDOMIZE_GATEX: std_randomize(Renato::GATEX_1); break;
        case RANDOMIZE_GATEY: std_randomize(Renato::GATEY_1); break;
        case RANDOMIZE_ACCESS: std_randomize(Renato::ACCESS_1); break;
    }
}

RenatoWidget::RenatoWidget()
{
    Renato *module = new Renato();
    setModule(module);
    box.size = Vec(27 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/RenatoModule.svg")));
    addChild(panel);
    addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));
    addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));

	int x = 20;
	int y = 30;
	int dist_h = 32;
    addInput(createInput<PJ301MPort>(Vec(x, y ), module, Renato::XCLK));
    x+= dist_h;
    addInput(createInput<PJ301MPort>(Vec(x, y ), module, Renato::YCLK));
    x+= 2*dist_h;

    // page 0 (SESSION)
    ParamWidget *pwdg = createParam<NKK2>(Vec(x,y-10), module, Renato::COUNTMODE_X, 0.0, 2.0, 0.0);
	addParam(pwdg);
    #ifdef LAUNCHPAD
    LaunchpadRadio *radio = new LaunchpadRadio(0, ILaunchpadPro::RC2Key(2,1), 3, LaunchpadLed::Color(47), LaunchpadLed::Color(32));
    module->drv->Add(radio, pwdg);
    #endif

	x+= 5*dist_h/3;
	pwdg = createParam<NKK2>(Vec(x,y-10), module, Renato::COUNTMODE_Y, 0.0, 2.0, 0.0);
	addParam(pwdg);
    #ifdef LAUNCHPAD
    radio = new LaunchpadRadio(0, ILaunchpadPro::RC2Key(2,3), 3, LaunchpadLed::Color(19), LaunchpadLed::Color(21));
    module->drv->Add(radio, pwdg);
    #endif

	x+= 5*dist_h/3;
	pwdg = createParam<NKK2>(Vec(x,y-10), module, Renato::SEEKSLEEP, 0.0, 1.0, 0.0);
	addParam(pwdg);
    #ifdef LAUNCHPAD
    radio = new LaunchpadRadio(0, ILaunchpadPro::RC2Key(2,5), 2, LaunchpadLed::Color(51), LaunchpadLed::Color(52));
    module->drv->Add(radio, pwdg);
    #endif

	x = box.size.x - 3 * dist_h-20;
    addOutput(createOutput<PJ301MPort>(Vec(x, y ), module, Renato::CV));
    x+= dist_h;
    addOutput(createOutput<PJ301MPort>(Vec(x, y ), module, Renato::XGATE));
    addChild(createLight<MediumLight<GreenLight>>(Vec(x+18, y +27), module, Renato::LED_GATEX ));
    x+= dist_h;
    addOutput(createOutput<PJ301MPort>(Vec(x, y ), module, Renato::YGATE));
    addChild(createLight<MediumLight<GreenLight>>(Vec(x+18, y +27), module, Renato::LED_GATEY ));

    // page 1 (NOTES)
	x = 40;
	y = 90;
	dist_h = 95;
	int dist_v = 75;
	for(int r=0; r < 4; r++)
	{
		for(int c=0; c < 4; c++)
		{
			int n = c+r*4;
			addParam(createParam<Davies1900hBlackKnob>(Vec(x+dist_h*c, y + dist_v*r), module, Renato::VOLTAGE_1+n, 0.005, 6.0, 1.0));

			pwdg = createParam<CKSS>(Vec(x+dist_h*c-18, y + dist_v*r+8), module, Renato::ACCESS_1+n, 0.0, 1.0, 1.0);
            addParam(pwdg);
            #ifdef LAUNCHPAD
            LaunchpadSwitch *pswitch = new LaunchpadSwitch(1, ILaunchpadPro::RC2Key(r+4,c), LaunchpadLed::Off(), LaunchpadLed::Color(17));
            module->drv->Add(pswitch, pwdg);
            #endif

            pwdg = createParam<CKSS>(Vec(x+dist_h*c+40, y + dist_v*r-12), module, Renato::GATEY_1+n, 0.0, 1.0, 1.0);
            addParam(pwdg);
            #ifdef LAUNCHPAD
            pswitch = new LaunchpadSwitch(1, ILaunchpadPro::RC2Key(r,c+4), LaunchpadLed::Off(), LaunchpadLed::Color(62));
            module->drv->Add(pswitch, pwdg);
            #endif

            pwdg = createParam<CKSS>(Vec(x+dist_h*c+40, y + dist_v*r+28), module, Renato::GATEX_1+n, 0.0, 1.0, 1.0);
            addParam(pwdg);
            #ifdef LAUNCHPAD
            pswitch = new LaunchpadSwitch(1, ILaunchpadPro::RC2Key(r+4,c+4), LaunchpadLed::Off(), LaunchpadLed::Color(52));
            module->drv->Add(pswitch, pwdg);
            #endif

            ModuleLightWidget *plight = createLight<LargeLight<RedLight>>(Vec(x+dist_h*c-4, y + dist_v*r+35), module, Renato::LED_1 + n);
            addChild(plight);
            #ifdef LAUNCHPAD
            LaunchpadLight *ld1 = new LaunchpadLight(1, ILaunchpadPro::RC2Key(r,c), LaunchpadLed::Off(), LaunchpadLed::Color(4));
            module->drv->Add(ld1, plight);
            #endif
		}
	}

    #ifdef LAUNCHPAD
    addChild(new DigitalLed((box.size.x-24)/2, 5, &module->connected));
    #endif
}
