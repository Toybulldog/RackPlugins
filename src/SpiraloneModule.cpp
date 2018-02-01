#include "Spiralone.hpp"
#include <math.h>

#define TOTAL_STEPS (32)
#define NUM_SEQUENCERS (6)

struct Spiralone : Module
{
    enum ParamIds
    {
        VOLTAGE_1,
        NUM_PARAMS = VOLTAGE_1 + TOTAL_STEPS
    };

    enum InputIds
    {


        NUM_INPUTS
    };

    enum OutputIds
    {

        NUM_OUTPUTS
    };

    enum LightIds
    {
        LED_SEQUENCE_1,
        NUM_LIGHTS = (LED_SEQUENCE_1 + TOTAL_STEPS) * NUM_SEQUENCERS
    };


    Spiralone() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
    {
        #ifdef LAUNCHPAD
        drv = new LaunchpadBindingDriver(Scene5, 3);
        drv->SetAutoPageKey(LaunchpadKey::SESSION, 0);
        drv->SetAutoPageKey(LaunchpadKey::NOTE, 1);
        drv->SetAutoPageKey(LaunchpadKey::DEVICE, 2);
        #endif
        on_loaded();
    }

    #ifdef LAUNCHPAD
    ~Spiralone()
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

	#ifdef LAUNCHPAD
    LaunchpadBindingDriver *drv;
    float connected;
    #endif

private:
    void on_loaded();
    void load();

};

void Spiralone::on_loaded()
{
    #ifdef LAUNCHPAD
    connected=0;
    #endif
    load();
}

void Spiralone::load()
{

}

void Spiralone::step()
{

    #ifdef LAUNCHPAD
    connected = drv->Connected() ? 1.0 : 0.0;
    drv->ProcessLaunchpad();
    #endif
}

SpiraloneWidget::SpiraloneWidget()
{
    Spiralone *module = new Spiralone();
    setModule(module);
    box.size = Vec(28 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    SVGPanel *panel = new SVGPanel();
    panel->box.size = box.size;
    panel->setBackground(SVG::load(assetPlugin(plugin, "res/SpiraloneModule.svg")));
    addChild(panel);
    addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
    addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
    addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));
    addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));

    int x = 190;
    int y = 190;
    float step = 2*M_PI / TOTAL_STEPS;
    float angle = M_PI/2.0;
    for(int k = 0; k < TOTAL_STEPS; k++)
    {
        int r = 160;
        float cx = cos(angle);
        float cy = sin(angle);
        angle -= step;
        addParam(createParam<BefacoTinyKnob>(Vec(x+r*cx-16.4, y+r*cy-16.4), module, Spiralone::VOLTAGE_1+k, 0.0, 6.0, 1.0));

        r -= 10;
        for(int s = 0; s < NUM_SEQUENCERS; s++)
        {
            int n = s * TOTAL_STEPS + k;
            r -= 16;
            addChild(createLight<SmallLight<RedLight>>(Vec(x+r*cx, y+r*cy), module, Spiralone::LED_SEQUENCE_1 + n));

            createSequencer(s);
        }
    }

    #ifdef LAUNCHPAD
    addChild(new DigitalLed((box.size.x-24)/2, 5, &module->connected));
    #endif
}

void SpiraloneWidget::createSequencer(int seq)
{

}


