#include "M581.hpp"
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
    int numSteps;
    int runMode;

private:
    bool testaCroce() {return rand() >= RAND_MAX/2;}
    int getRand(int rndMax)
    {   float f = rand()/float(RAND_MAX);
        f *= rndMax;
        return int(f);
    }
    void reset_counters()
    {
        pulseCounter=0;
        for(curStep = 0; curStep < 8; curStep++)
        {
            if(params[STEP_ENABLE+curStep].value > 0.0)  // step on?
                break;
        }

        pp_rev=false;
    }
    void on_loaded();
    bool pp_rev;
    int get_next_step(int current);
    void playCurrent(float deltaTime);
    void showCurStep();
    int inc_step(int step);
    int dec_step(int step);
    bool any();
    SchmittTrigger clockTrigger;
    SchmittTrigger resetTrigger;
    int curStep;
    int pulseCounter;
};

void M581::on_loaded()
{
    numSteps = std::round(params[NUM_STEPS].value);
    runMode = std::round(params[RUN_MODE].value);
    reset_counters();
}

void M581::step()
{
    float deltaTime = 1.0 / engineGetSampleRate();
    numSteps = std::round(params[NUM_STEPS].value);
    runMode = std::round(params[RUN_MODE].value);
    if(resetTrigger.process(inputs[RESET].value))
    {
        reset_counters();
    } else if(clockTrigger.process(inputs[CLOCK].value) && any())
    {
        playCurrent(deltaTime);

    }
}

void M581::playCurrent(float deltaTime)
{

    if(++pulseCounter > std::round(params[COUNTER_SWITCH+curStep].value))
    {
        pulseCounter = 0;
        curStep = get_next_step(curStep);
    }
    showCurStep();
}

void M581::showCurStep()
{
    int lled = curStep % 8;
    for(int k=0; k < 8; k++)
        lights[LED_STEP+k].value = k==lled? 1.0 : 0.0;
}

int M581::inc_step(int step)
{
    for(int k = 0; k < 8; k++)
    {
        if(++step >= numSteps)
            step = 0;
        if(params[STEP_ENABLE+step].value > 0.0)  // step on?
            break;
    }

    return step;
}

int M581::dec_step(int step)
{
    for(int k = 0; k < 8; k++)
    {
        if(--step < 0)
            step =numSteps - 1;
        if(params[STEP_ENABLE+step].value > 0.0)  // step on?
            break;
    }

    return step;
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

int M581::get_next_step(int current)
{
    switch(runMode)
    {
        case 0: // FWD
            return inc_step(current);

        case 1: // BWD
            return dec_step(current);

        case 2: // ping ed anche pong
            if(pp_rev)
            {
                int step = dec_step(current);
                if( step <= current )
                    return step;
                pp_rev = !pp_rev;
                return inc_step(current);
            } else
            {
                int step = inc_step(current);
                if(step >= current )
                    return step;
                pp_rev = !pp_rev;
                return dec_step(current);
            }
            break;

        case 3: // BROWNIAN
        {
            if(testaCroce())
            {
                return inc_step(current);
            } else
            {
                return testaCroce() ? dec_step(current) : current;
            }
        }
        break;

        case 4: // At casacc
            current = getRand(numSteps); // OCIO: step off NON funziona con random!
            break;
    }

    return current;
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
    display2->value = &module->numSteps;
    addChild(display2);
    addParam(createParam<BefacoTinyKnob>(Vec(312, RACK_GRID_HEIGHT-123), module, M581::NUM_STEPS,1.0, 31.0, 8.0));

    // run mode
    RunModeDisplay *display = new RunModeDisplay();
    display->box.pos = Vec(346, RACK_GRID_HEIGHT-63);
    display->box.size = Vec(42, 20);
    display->mode = &module->runMode;
    addChild(display);
    addParam(createParam<BefacoTinyKnob>(Vec(312, RACK_GRID_HEIGHT-66), module, M581::RUN_MODE,0.0, 4.0, 0.0));
}
