#include "M581.hpp"
#include "M581Types.hpp"
#include <sstream>

struct M581 : Module
{
    enum ParamIds
    {
        GATE_SWITCH,
        COUNTER_SWITCH=GATE_SWITCH+8,
        STEP_NOTES=COUNTER_SWITCH+8,
        STEP_ENABLE=STEP_NOTES+8,
        GATE_TIME=STEP_ENABLE+8,
        SLIDE_TIME,
        NUM_STEPS,
        RUN_MODE,
        STEP_DIV,
        MAXVOLTS
        ,NUM_PARAMS
    };

    enum InputIds
    {
        CLOCK,
        RESET,
        NUM_INPUTS
    };

    enum OutputIds
    {
        CV,
        GATE,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        LED_STEP,
        LED_SUBDIV=LED_STEP+8,
        temp=LED_SUBDIV+8,
        NUM_LIGHTS
    };

    M581() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
    {
        #if ARCH_WIN
        drv = new LaunchpadBindingDriver(Scene2, 3);
        drv->SetAutoPageKey(LaunchpadKey::SESSION, 0);
        drv->SetAutoPageKey(LaunchpadKey::NOTE, 1);
        drv->SetAutoPageKey(LaunchpadKey::DEVICE, 2);
        #endif
        on_loaded();
    }

    ~M581()
    {
        #if ARCH_WIN
        delete drv;
        #endif // ARCH_WIN
    }

    void step() override;
    void reset() override {on_loaded();}
    void randomize() override {on_loaded();}

    void fromJson(json_t *root) override {Module::fromJson(root); on_loaded();}
    json_t *toJson() override
    {
		json_t *rootJ = json_object();

		return rootJ;
	}

	float *getAddress(int var)
	{
	    switch(var)
	    {
	        case 0: return &params[M581::RUN_MODE].value;
	        case 1: return &params[M581::NUM_STEPS].value;
	    }
	    return NULL;
	}

	#if ARCH_WIN
    LaunchpadBindingDriver *drv;
    float connected;
    #endif

private:
	CV_LINE cvControl;
	GATE_LINE gateControl;
	TIMER Timer;
	STEP_COUNTER stepCounter;
	ParamGetter getter;

	void _reset();
    void on_loaded();
    void beginNewStep();
    void showCurStep(int cur_step, int sub_div);
    bool any();
    SchmittTrigger clockTrigger;
    SchmittTrigger resetTrigger;
};

void M581::on_loaded()
{
    #if ARCH_WIN
    connected=0;
    #endif
    stepCounter.Set(&getter);
    cvControl.Set(&getter);
    gateControl.Set(&getter);
    getter.Set(this);
    _reset();
}

void M581::_reset()
{
	cvControl.Reset();
	gateControl.Reset();
    stepCounter.Reset(&Timer);
    showCurStep(0, 0);
}

void M581::step()
{
    if(resetTrigger.process(inputs[RESET].value))
    {
        _reset();
    } else
    {
        Timer.Step(); // return deltaTime

        if(clockTrigger.process(inputs[CLOCK].value) && any())
            beginNewStep();

        outputs[CV].value = cvControl.Play(Timer.Elapsed());
        outputs[GATE].value = gateControl.Play(&Timer, stepCounter.PulseCounter());
    }

    #if ARCH_WIN
    connected = drv->Connected() ? 1.0 : 0.0;
    drv->ProcessLaunchpad();
    #endif
}

void M581::beginNewStep()
{
	int cur_step;
    if(stepCounter.Play(&Timer, &cur_step)) // inizia un nuovo step?
    {
		gateControl.Begin(cur_step);
		cvControl.Begin(cur_step);	// 	glide note increment in 1/10 di msec. param = new note value
    }

    showCurStep(cur_step, stepCounter.PulseCounter());
}

void M581::showCurStep(int cur_step, int sub_div)
{
    int lled = cur_step;
    int sled = sub_div;
    for(int k=0; k < 8; k++)
    {
        lights[LED_STEP+k].value = k==lled ? 1.0 : 0.0;
        lights[LED_SUBDIV+k].value = k==sled ? 1.0 : 0.0;
    }
}

bool M581::any()
{
    for(int k = 0; k < 8; k++)
    {
        if(getter.IsEnabled(k))  // step on?
            return true;
    }

    return false;
}

M581Widget::M581Widget()
{
    M581 *module = new M581();
    setModule(module);
    box.size = Vec(27 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/M581Module.svg")));
    addChild(panel);
    addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));
    addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));

    for(int k=0; k< 8; k++)
    {
   // page #0 (Session): step enable/disable; gate mode
         // step enable
        ParamWidget *pwdg = createParam<CKSSThree>(Vec(36 + 35 * k, RACK_GRID_HEIGHT-58), module, M581::STEP_ENABLE + k, 0.0, 2.0, 1.0);
        addParam(pwdg);
        #if ARCH_WIN
        LaunchpadRadio *radio = new LaunchpadRadio(0, ILaunchpadPro::RC2Key(5,k), 3, LaunchpadLed::Color(2), LaunchpadLed::Color(3));
        module->drv->Add(radio, pwdg);
        #endif

        // Gate switches
        pwdg = createParam<GateSwitch>(Vec(39 + 35 * k, RACK_GRID_HEIGHT-140), module, M581::GATE_SWITCH + k, 0.0, 3.0, 2.0);
        addParam(pwdg);
        #if ARCH_WIN
        radio = new LaunchpadRadio(0, ILaunchpadPro::RC2Key(1,k), 4, LaunchpadLed::Color(6), LaunchpadLed::Color(8));
        module->drv->Add(radio, pwdg);
        #endif

        // page #1 (Note): Notes
        // step notes
        pwdg = createParam<BefacoSlidePot>(Vec(35 + 35 * k, RACK_GRID_HEIGHT-368), module, M581::STEP_NOTES + k, 0.001, 1.0, 0.5);
        addParam(pwdg);
        #if ARCH_WIN
        LaunchpadKnob *pknob = new LaunchpadKnob(1, ILaunchpadPro::RC2Key(6,k), LaunchpadLed::Rgb(20,10,10), LaunchpadLed::Rgb(60,40,40));
        module->drv->Add(pknob, pwdg);
        #endif // ARCH_WIN

        //page #2 (Device): Counters
        // Counter switches
        pwdg = createParam<CounterSwitch>(Vec(39 + 35 * k, RACK_GRID_HEIGHT-246), module, M581::COUNTER_SWITCH + k, 0.0, 7.0, 0.0);
        addParam(pwdg);
        #if ARCH_WIN
        radio = new LaunchpadRadio(2, ILaunchpadPro::RC2Key(0,k), 8, LaunchpadLed::Color(6), LaunchpadLed::Color(8));
        module->drv->Add(radio, pwdg);
        #endif

        // step leds (all pages)
        ModuleLightWidget *plight = createLight<LargeLight<RedLight>>(Vec(36 + 35 * k, RACK_GRID_HEIGHT-80), module, M581::LED_STEP + k);
        addChild(plight);
        #if ARCH_WIN
        LaunchpadLight *ld1 = new LaunchpadLight(launchpadDriver::ALL_PAGES, ILaunchpadPro::RC2Key(0,k), LaunchpadLed::Off(), LaunchpadLed::Color(63));
        module->drv->Add(ld1, plight);
        #endif // ARCH_WIN

        // subdiv leds (all pages)
        plight = createLight<TinyLight<RedLight>>(Vec(26, RACK_GRID_HEIGHT-162 - 11.28 * k), module, M581::LED_SUBDIV + k);
        addChild(plight);
        #if ARCH_WIN
        ld1 = new LaunchpadLight(launchpadDriver::ALL_PAGES, ILaunchpadPro::RC2Key(k,8), LaunchpadLed::Off(), LaunchpadLed::Color(61));   // colonna PLAY
        module->drv->Add(ld1, plight);
        #endif
     }

    // Gate time
    addParam(createParam<Davies1900hBlackKnob>(Vec(310, RACK_GRID_HEIGHT-368), module, M581::GATE_TIME, 0.005, 1.0, 0.25));    // in sec
    // Slide time
    addParam(createParam<Davies1900hBlackKnob>(Vec(310, RACK_GRID_HEIGHT-310), module, M581::SLIDE_TIME, 0.005, 2.0, 0.5)); // in sec

    // volt fondo scala
    addParam(createParam<CKSS>(Vec(12, RACK_GRID_HEIGHT-350), module, M581::MAXVOLTS, 0.0, 1.0, 1.0));

    // step div
    addParam(createParam<GateSwitch>(Vec(364, RACK_GRID_HEIGHT-340), module, M581::STEP_DIV, 0.0, 3.0, 0.0));

      // input
    addInput(createInput<PJ301MPort>(Vec(320, RACK_GRID_HEIGHT-240), module, M581::RESET));
    addInput(createInput<PJ301MPort>(Vec(360, RACK_GRID_HEIGHT-240), module, M581::CLOCK));

      // OUTPUTS
    addOutput(createOutput<PJ301MPort>(Vec(320, RACK_GRID_HEIGHT-184), module, M581::CV));
    addOutput(createOutput<PJ301MPort>(Vec(360, RACK_GRID_HEIGHT-184), module, M581::GATE));

    // # STEPS
    SigDisplayWidget *display2 = new SigDisplayWidget();
    display2->box.pos = Vec(346, RACK_GRID_HEIGHT-120);
    display2->box.size = Vec(30, 20);
    display2->value = module->getAddress(1);
    addChild(display2);
    addParam(createParam<BefacoSnappedTinyKnob>(Vec(312, RACK_GRID_HEIGHT-123), module, M581::NUM_STEPS,1.0, 31.0, 8.0));

    // run mode
    RunModeDisplay *display = new RunModeDisplay();
    display->box.pos = Vec(346, RACK_GRID_HEIGHT-63);
    display->box.size = Vec(42, 20);
    display->mode = module->getAddress(0);
    addChild(display);
    addParam(createParam<BefacoSnappedTinyKnob>(Vec(312, RACK_GRID_HEIGHT-66), module, M581::RUN_MODE,0.0, 4.0, 0.0));

    #if ARCH_WIN
    addChild(new DigitalLed(360, 20, &module->connected));
    #endif // ARCH_WIN
}

Menu *M581Widget::createContextMenu()
{
	Menu *menu = ModuleWidget::createContextMenu();

	MenuLabel *spacerLabel = new MenuLabel();
	menu->addChild(spacerLabel);

	menu->addChild(new KleeMenuItem("Randomize Pitch", this, RANDOMIZE_PITCH));
	menu->addChild(new KleeMenuItem("Randomize Counters", this, RANDOMIZE_COUNTER));
	menu->addChild(new KleeMenuItem("Randomize Modes", this, RANDOMIZE_MODE));
	menu->addChild(new KleeMenuItem("Randomize Enable/Slide", this, RANDOMIZE_ENABLE));
	return menu;
}

void M581Widget::onMenu(MENUACTIONS action)
{
    switch(action)
    {
        case RANDOMIZE_COUNTER: std_randomize(M581::COUNTER_SWITCH); break;
        case RANDOMIZE_PITCH: std_randomize(M581::STEP_NOTES); break;
        case RANDOMIZE_MODE: std_randomize(M581::GATE_SWITCH); break;
        case RANDOMIZE_ENABLE: std_randomize(M581::STEP_ENABLE); break;
    }
}

bool ParamGetter::IsEnabled(int numstep) {return pModule->params[M581::STEP_ENABLE + numstep].value > 0.0;}
bool ParamGetter::IsSlide(int numstep) {return pModule->params[M581::STEP_ENABLE + numstep].value > 1.0;}
int ParamGetter::GateMode(int numstep) {return std::round(pModule->params[M581::GATE_SWITCH + numstep].value);}
int ParamGetter::PulseCount(int numstep) {return std::round(pModule->params[M581::COUNTER_SWITCH + numstep].value);}
float ParamGetter::Note(int numstep) {return pModule->params[M581::STEP_NOTES + numstep].value * (pModule->params[M581::MAXVOLTS].value > 0 ? 5.0 : 3.0);}
int ParamGetter::RunMode() {return std::round(pModule->params[M581::RUN_MODE].value);}
int ParamGetter::NumSteps() {return std::round(pModule->params[M581::NUM_STEPS].value);}
float ParamGetter::SlideTime() {return pModule->params[M581::SLIDE_TIME].value;}
float ParamGetter::GateTime() {return pModule->params[M581::GATE_TIME].value;}
int ParamGetter::StepDivision() {return std::round(pModule->params[M581::STEP_DIV].value) + 1;}
