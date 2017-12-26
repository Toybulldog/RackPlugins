#include "Klee.hpp"

struct Klee : Module
{
    enum ParamIds
    {
        PITCH_KNOB,
        GROUPBUS = PITCH_KNOB + 16,
        LOAD_BUS = GROUPBUS + 16,
        LOAD_PARAM = LOAD_BUS + 16,
        STEP_PARAM,
        X28_X16,
        RND_PAT,
        B_INV,
        RND_THRESHOLD,
        BUS1_LOAD,
        BUS_MERGE,
        RANGE = BUS_MERGE + 3,
        NUM_PARAMS
    };

    enum InputIds
    {
        LOAD_INPUT,
        EXT_CLOCK_INPUT,
        RND_THRES_IN,
        RANGE_IN,
        NUM_INPUTS
    };

    enum OutputIds
    {
        CV_A,
        CV_B,
        CV_AB,
        CV_A__B,
        GATE_OUT,
        TRIG_OUT = GATE_OUT + 3,
        temp = TRIG_OUT + 3,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        LED_PITCH,

        LED_BUS = LED_PITCH + 16,
        temp1 = LED_BUS + 3,
        NUM_LIGHTS
    };

    Klee() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
    void reset() override {load();}
    void randomize() override {load();}

private:
    const float pulseTime = 0.002;      //2msec trigger
    const float LVL_ON = 10.0;
    const float LVL_OFF = 0.0;
    void showValues();
    void sr_rotate();
    bool chance();
    void populate_gate(int clk);
    void update_bus();
    void load();
    void populate_outputs();
    void check_triggers(float deltaTime);
    bool isSwitchOn(int ptr);
    int getValue3(int k);
    SchmittTrigger loadTrigger;
    SchmittTrigger2 clockTrigger;
    PulseGenerator triggers[3];

    union
    {
        struct
        {
            bool A[8];
            bool B[8];
        };
        bool P[16];
    } shiftRegister;

    bool bus_active[3];
};

void Klee::step()
{
    float deltaTime = 1.0 / engineGetSampleRate();

    if (loadTrigger.process(params[LOAD_PARAM].value + inputs[LOAD_INPUT].value))
    {
        load();
    }

    int clk = clockTrigger.process(inputs[EXT_CLOCK_INPUT].value + params[STEP_PARAM].value); // 1=rise, -1=fall
    if (clk == 1)
    {
        sr_rotate();
        update_bus();
        populate_outputs();
    }

    if (clk != 0)
    {
        populate_gate(clk);
    }

    check_triggers(deltaTime);

    showValues();
}

void Klee::load()
{
    for (int k = 0; k < 16; k++)
    {
        shiftRegister.P[k] = isSwitchOn(LOAD_BUS + k);
    }
}


void Klee::update_bus()
{
    bool bus1 = bus_active[0];
    for(int k = 0; k < 3;k++)
        bus_active[k] = false;

    for (int k = 0; k < 16; k++)
    {
        if (shiftRegister.P[k])
        {
            bus_active[getValue3(k)] = true;
        }
    }
    bus_active[1] &= !(bus_active[0] || bus_active[2]);  //BUS 2: NOR 0 , 3

    //bus1 load
    if (isSwitchOn(BUS1_LOAD) && !bus1 && bus_active[0])
        load();
}

int Klee::getValue3(int k)
{
    if(params[GROUPBUS + k].value < 0.5) return 2;
    if(params[GROUPBUS + k].value > 1.0) return 0;
    return 1;
}

bool Klee::isSwitchOn(int ptr)
{
    return params[ptr].value >= 0.5;
}

void Klee::check_triggers(float deltaTime)
{
    for (int k = 0; k < 3; k++)
    {
        if (outputs[TRIG_OUT + k].value > 0.5 && !triggers[k].process(deltaTime))
        {
            outputs[TRIG_OUT + k].value = LVL_OFF;
        }
    }
}

void Klee::populate_gate(int clk)
{
    for (int k = 0; k < 3; k++)
    {
        // gate
        if (clk == 1)  // rise
        {
            outputs[GATE_OUT + k].value = bus_active[k] ? LVL_ON : LVL_OFF;
        }
        else // fall
        {
            if (!bus_active[k] || !isSwitchOn(BUS_MERGE + k))
                outputs[GATE_OUT + k].value = LVL_OFF;
        }
    }
}

void Klee::populate_outputs()
{
    for (int k = 0; k < 3; k++)
    {
        if (bus_active[k])
        {
            outputs[TRIG_OUT + k].value = LVL_ON;
            triggers[k].trigger(pulseTime);
        }
    }

    float a = 0, b = 0;
    float mult = params[RANGE].value + inputs[RANGE_IN].value;

    for (int k = 0; k < 8; k++)
    {
        if (shiftRegister.A[k])
            a += params[PITCH_KNOB + k].value*mult;

        if (shiftRegister.B[k])
            b += params[PITCH_KNOB + k + 8].value*mult;
    }
    outputs[CV_A].value = a;
    outputs[CV_B].value = b;
    outputs[CV_AB].value = a + b;
    outputs[CV_A__B].value = a - b;
}

void Klee::showValues()
{
    for (int k = 0; k < 16; k++)
    {
        lights[LED_PITCH + k].value = shiftRegister.P[k] ? 1.0 : 0;
    }

    for (int k = 0; k < 3; k++)
    {
        lights[LED_BUS + k].value = outputs[GATE_OUT + k].value;
    }
}

void Klee::sr_rotate()
{
    if (!isSwitchOn(X28_X16))  // mode 1 x 16
    {
        int fl = shiftRegister.P[15];
        for (int k = 15; k > 0; k--)
        {
            shiftRegister.P[k] = shiftRegister.P[k - 1];
        }
        if (isSwitchOn(RND_PAT))
            shiftRegister.P[0] = chance();
        else
            shiftRegister.P[0] = fl;
    }
    else
    {
        int fla = shiftRegister.A[7];
        int flb = shiftRegister.B[7];
        for (int k = 7; k > 0; k--)
        {
            shiftRegister.A[k] = shiftRegister.A[k - 1];
            shiftRegister.B[k] = shiftRegister.B[k - 1];
        }
        if (isSwitchOn(RND_PAT))
            shiftRegister.A[0] = chance();
        else
            shiftRegister.A[0] = fla;
        shiftRegister.B[0] = isSwitchOn(B_INV) ? !flb : flb;
    }
}

bool Klee::chance()
{
    return rand() <= (params[RND_THRESHOLD].value + inputs[RND_THRES_IN].value) * RAND_MAX;
}

KleeWidget::KleeWidget()
{
    Klee *module = new Klee();
    setModule(module);
    box.size = Vec(48 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/KleeModule.svg")));
    addChild(panel);
    addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));
    addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));

    for(int k=0; k< 8; k++)
    {
        // Load switches
        addParam(createParam<NKK2>(Vec(38 + 35 * k, RACK_GRID_HEIGHT-370), module, Klee::LOAD_BUS + k, 0.0, 1.0, 0.0));
        addParam(createParam<NKK2>(Vec(343 + 35 * k, RACK_GRID_HEIGHT-370), module, Klee::LOAD_BUS + k + 8, 0.0, 1.0, 0.0));

        // BUS switches
        addParam(createParam<NKK3>(Vec(19 + 35 * k, RACK_GRID_HEIGHT-55), module, Klee::GROUPBUS + k, 0.0, 2.0, 2.0));
        addParam(createParam<NKK3>(Vec(356 + 35 * k, RACK_GRID_HEIGHT-55), module, Klee::GROUPBUS + k + 8, 0.0, 2.0, 2.0));
    }

     // trig/gate out
    for(int k = 0; k < 3; k++)
    {
        addParam(createParam<NKK2>(Vec(572, RACK_GRID_HEIGHT-202-28+k*54), module, Klee::BUS_MERGE + k, 0.0, 1.0, 0.0));
        addChild(createLight<LargeLight<BlueLight>>(Vec(616, RACK_GRID_HEIGHT-187-28+k*54), module, Klee::LED_BUS + k));
        addOutput(createOutput<PJ301MPort>(Vec(648, RACK_GRID_HEIGHT-192-28+k*54), module, Klee::TRIG_OUT + k));
        addOutput(createOutput<PJ301MPort>(Vec(688, RACK_GRID_HEIGHT-192-28+k*54), module, Klee::GATE_OUT + k));
    }

    //load
    addParam(createParam<BefacoPush>(Vec(10, RACK_GRID_HEIGHT-296), module, Klee::LOAD_PARAM, 0.0, 1.0, 0.0));
    addInput(createInput<PJ301MPort>(Vec(12, RACK_GRID_HEIGHT-28-210), module, Klee::LOAD_INPUT));
    addParam(createParam<NKK2>(Vec(57, RACK_GRID_HEIGHT-304), module, Klee::BUS1_LOAD, 0.0, 1.0, 0.0));

    //step
    addParam(createParam<BefacoPush>(Vec(10, RACK_GRID_HEIGHT-132-28), module, Klee::STEP_PARAM, 0.0, 1.0, 0.0));
    addInput(createInput<PJ301MPort>(Vec(12, RACK_GRID_HEIGHT-76-28), module, Klee::EXT_CLOCK_INPUT));

    // CV Out
    addOutput(createOutput<PJ301MPort>(Vec(643, RACK_GRID_HEIGHT-333-28), module, Klee::CV_A));
    addOutput(createOutput<PJ301MPort>(Vec(687, RACK_GRID_HEIGHT-333-28), module, Klee::CV_B));
    addOutput(createOutput<PJ301MPort>(Vec(687, RACK_GRID_HEIGHT-288-28), module, Klee::CV_AB));
    addOutput(createOutput<PJ301MPort>(Vec(643, RACK_GRID_HEIGHT-288-28), module, Klee::CV_A__B));

    // mode
    addParam(createParam<NKK2>(Vec(258, RACK_GRID_HEIGHT-182-28), module, Klee::X28_X16, 0.0, 1.0, 0.0));     // 2x8 1x16
    addParam(createParam<NKK2>(Vec(310, RACK_GRID_HEIGHT-182-28), module, Klee::RND_PAT, 0.0, 1.0, 0.0));     // rnd/pattern
    addParam(createParam<NKK2>(Vec(362, RACK_GRID_HEIGHT-182-28), module, Klee::B_INV, 0.0, 1.0, 0.0));     // norm /B inverted

    // CV Range
    addParam(createParam<Davies1900hBlackKnob>(Vec(57, RACK_GRID_HEIGHT-138-28), module, Klee::RANGE, 0.0, 5.0, 1.0));
    addInput(createInput<PJ301MPort>(Vec(62, RACK_GRID_HEIGHT-76-28), module, Klee::RANGE_IN));

    // RND Threshold
    addParam(createParam<Davies1900hBlackKnob>(Vec(535, RACK_GRID_HEIGHT-276-28), module, Klee::RND_THRESHOLD, 0.0, 1.0, 0.0));     // rnd threshold
    addInput(createInput<PJ301MPort>(Vec(584, RACK_GRID_HEIGHT-270-28), module, Klee::RND_THRES_IN));


    // pitch Knobs + leds
    int pos_x[8] = {109, 143, 202, 270, 336, 401, 461, 496};
    int pos_y[8] = {232, 272, 299, 307, 307, 299, 272, 232};
    for(int k = 0; k < 8; k++)
    {
        addParam(createParam<Davies1900hBlackKnob>(Vec(pos_x[k], RACK_GRID_HEIGHT-pos_y[k]), module, Klee::PITCH_KNOB + k, 0.0, 1.0, 0.0));
        addChild(createLight<MediumLight<RedLight>>(Vec(pos_x[k]+38, RACK_GRID_HEIGHT-pos_y[k]+20), module, Klee::LED_PITCH + k));
        addParam(createParam<Davies1900hBlackKnob>(Vec(pos_x[7-k], RACK_GRID_HEIGHT-419+pos_y[7-k]), module, Klee::PITCH_KNOB + 8+ k, 0.0, 1.0, 0.0));
        addChild(createLight<MediumLight<GreenLight>>(Vec(pos_x[7-k]-12, RACK_GRID_HEIGHT-419+pos_y[7-k]+20), module, Klee::LED_PITCH + k+8));
    }
}
