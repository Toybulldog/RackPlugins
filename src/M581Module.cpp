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
        temp=LED_STEP+8,
        NUM_LIGHTS
    };

    M581() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
    {
        on_loaded();
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
    int *getAddress(int var)  {return stepCounter.getAddress(var);}

private:
	CV_LINE cvControl;
	GATE_LINE gateControl;
	TIMER Timer;
	STEP_COUNTER stepCounter;

	void _reset();
    void on_loaded();
    void doTimeouts(bool play, float deltaTime);
    void playCurrent(float deltaTime);
    void showCurStep(int cur_step);
    bool any();
    SchmittTrigger clockTrigger;
    SchmittTrigger resetTrigger;
};

void M581::on_loaded()
{
    stepCounter.Set(std::round(params[RUN_MODE].value), std::round(params[NUM_STEPS].value));
    _reset();
}

void M581::_reset()
{
	Timer.Reset();
	cvControl.Reset();
	gateControl.Reset();
    stepCounter.Reset();
}

void M581::step()
{
    float deltaTime = Timer.Step();
	stepCounter.Set(std::round(params[RUN_MODE].value), std::round(params[NUM_STEPS].value));

    if(resetTrigger.process(inputs[RESET].value))
    {
        _reset();
    } else if(clockTrigger.process(inputs[CLOCK].value) && any())
    {
        playCurrent(deltaTime);
    } else
        doTimeouts(false, deltaTime);
}

void M581::playCurrent(float deltaTime)
{
	int cur_step = stepCounter.CurStep();
	bool play = stepCounter.Play(std::round(params[COUNTER_SWITCH+cur_step].value), &params[STEP_ENABLE]);
    if(play)
    {
		gateControl.Set(params[GATE_TIME].value, std::round(params[STEP_DIV].value));
		cvControl.Set(params[STEP_NOTES + cur_step].value, params[SLIDE_TIME].value, params[STEP_ENABLE + cur_step].value > 1.0);	// 	glide note increment in 1/10 di msec. param = new note value
    }
    doTimeouts(play, deltaTime);
    showCurStep(cur_step);
}

void M581::doTimeouts(bool play, float deltaTime)
{
	outputs[CV].value = cvControl.Play(deltaTime, params[STEP_NOTES + stepCounter.CurStep()].value);
	gateControl.Play(play, std::round(params[GATE_SWITCH].value), deltaTime, &outputs[GATE].value);
}

void M581::showCurStep(int cur_step)
{
    int lled = cur_step % 8;
    for(int k=0; k < 8; k++)
        lights[LED_STEP+k].value = k==lled ? 1.0 : 0.0;
}

bool M581::any()
{
    for(int k = 0; k < 8; k++)
    {
        if(params[STEP_ENABLE+k].value > 0.0)  // step on?
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
         // step enable
        addParam(createParam<CKSSThree>(Vec(38 + 35 * k, RACK_GRID_HEIGHT-58), module, M581::STEP_ENABLE + k, 0.0, 2.0, 1.0));
        // step leds
        addChild(createLight<LargeLight<RedLight>>(Vec(38 + 35 * k, RACK_GRID_HEIGHT-80), module, M581::LED_STEP + k));
        // Gate switches
        addParam(createParam<GateSwitch>(Vec(37 + 35 * k, RACK_GRID_HEIGHT-140), module, M581::GATE_SWITCH + k, 0.0, 3.0, 0.0));
        // Counter switches
        addParam(createParam<CounterSwitch>(Vec(37 + 35 * k, RACK_GRID_HEIGHT-250), module, M581::COUNTER_SWITCH + k, 0.0, 7.0, 0.0));

        // step notes
        addParam(createParam<BefacoSlidePot>(Vec(38 + 35 * k, RACK_GRID_HEIGHT-368), module, M581::STEP_NOTES + k, 0.0, 5.0, 0.001));
    }

    // Gate time
    addParam(createParam<Davies1900hBlackKnob>(Vec(310, RACK_GRID_HEIGHT-368), module, M581::GATE_TIME, 0.05, 1.0, 0.5));    // in sec
    // Slide time
    addParam(createParam<Davies1900hBlackKnob>(Vec(310, RACK_GRID_HEIGHT-310), module, M581::SLIDE_TIME, 0.05, 1.0, 0.5)); // in sec

    // volt fondo scala
    addParam(createParam<CKSS>(Vec(12, RACK_GRID_HEIGHT-350), module, M581::MAXVOLTS, 3.0, 5.0, 3.0));

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
    addParam(createParam<BefacoTinyKnob>(Vec(312, RACK_GRID_HEIGHT-123), module, M581::NUM_STEPS,1.0, 31.0, 8.0));

    // run mode
    RunModeDisplay *display = new RunModeDisplay();
    display->box.pos = Vec(346, RACK_GRID_HEIGHT-63);
    display->box.size = Vec(42, 20);
    display->mode = module->getAddress(0);
    addChild(display);
    addParam(createParam<BefacoTinyKnob>(Vec(312, RACK_GRID_HEIGHT-66), module, M581::RUN_MODE,0.0, 4.0, 0.0));
}

