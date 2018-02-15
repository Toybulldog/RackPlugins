#include "Spiralone.hpp"
#include "sprlnSequencer.hpp"
#include "SpiraloneModule.hpp"
#include <math.h>

float AccessParam(Spiralone *p, int seq, int id) { return p->params[id + seq].value; }
float AccessParam(Spiralone *p, int id) { return p->params[id].value; }
Input *AccessInput(Spiralone *p, int seq, int id) { return &p->inputs[id + seq]; }
float *AccessOutput(Spiralone *p, int seq, int id) { return &p->outputs[id + seq].value; }
float *AccessLight(Spiralone *p, int id) { return &p->lights[id].value; }

void Spiralone::on_loaded()
{
#ifdef LAUNCHPAD
	connected = 0;
#endif
	load();
}

void Spiralone::load()
{
	for(int k = 0; k < NUM_SEQUENCERS; k++)
		sequencer[k].Reset(k, this);
}

void Spiralone::step()
{
	for(int k = 0; k < NUM_SEQUENCERS; k++)
		sequencer[k].Step(k, this);

#ifdef LAUNCHPAD
	connected = drv->Connected() ? 1.0 : 0.0;
	drv->ProcessLaunchpad();
#endif
}

SpiraloneWidget::SpiraloneWidget()
{
	color[0] = COLOR_RED;
	color[1] = COLOR_WHITE;
	color[2] = COLOR_BLUE;
	color[3] = COLOR_YELLOW;
	color[4] = COLOR_GREEN;

	Spiralone *module = new Spiralone();
	setModule(module);
	box.size = Vec(51 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
	SVGPanel *panel = new SVGPanel();
	panel->box.size = box.size;
	panel->setBackground(SVG::load(assetPlugin(plugin, "res/SpiraloneModule.svg")));

	addChild(panel);
	addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, box.size.y - RACK_GRID_WIDTH)));

	int x = 180;
	int y = 180;
	float step = 2 * M_PI / TOTAL_STEPS;
	float angle = -M_PI / 2.0;
	for(int k = 0; k < TOTAL_STEPS; k++)
	{
		int r = 160;
		float cx = cos(angle);
		float cy = sin(angle);
		angle += step;
		addParam(createParam<BefacoTinyKnob>(Vec(x + r * cx, y + r * cy), module, Spiralone::VOLTAGE_1 + k, 0.0, 6.0, 1.0));

		r -= 8;
		for(int s = 0; s < NUM_SEQUENCERS; s++)
		{
			int n = s * TOTAL_STEPS + k;
			r -= 15;
			addChild(createLed(s, Vec(x + r * cx + 9.7, y + r * cy + 9.4), module, Spiralone::LED_SEQUENCE_1 + n));

			createSequencer(s);
		}
	}

#ifdef LAUNCHPAD
	addChild(new DigitalLed((box.size.x - 28) / 2 - 32, RACK_GRID_HEIGHT - 28, &module->connected));
#endif
}

void SpiraloneWidget::createSequencer(int seq)
{
	int margin = 6;
	int x = 400;
	int y = (RACK_GRID_HEIGHT) / NUM_SEQUENCERS;

	y *= seq;
	y += 3 * margin + 2;

	addInput(createInput<PJ301RPort>(Vec(x, y - 11), module, Spiralone::CLOCK_1 + seq));
	addInput(createInput<PJ301YPort>(Vec(x, y + 19), module, Spiralone::RESET_1 + seq));
	x += 34;

	ParamWidget *pwdg = createParam<BefacoSnappedSwitch>(Vec(x, y), module, Spiralone::MODE_1 + seq, 0.0, 1.0, 0.0);
	addParam(pwdg);
#ifdef LAUNCHPAD
	int color_launchpad[NUM_SEQUENCERS][2];
	color_launchpad[0][0] = 11; color_launchpad[0][1] = 5;
	color_launchpad[1][0] = 1; color_launchpad[1][1] = 3;
	color_launchpad[2][0] = 47; color_launchpad[2][1] = 37;
	color_launchpad[3][0] = 15; color_launchpad[3][1] = 12;
	color_launchpad[4][0] = 19; color_launchpad[4][1] = 21;
	LaunchpadRadio *sw = new LaunchpadRadio(0, 0, ILaunchpadPro::RC2Key(0, seq), 2, LaunchpadLed::Color(color_launchpad[seq][0]), LaunchpadLed::Color(color_launchpad[seq][1]));
	((Spiralone *)module)->drv->Add(sw, pwdg);
#endif
	x += 50;
	addParam(createParam<BefacoSnappedTinyKnob>(Vec(x - 10, y - 12), module, Spiralone::LENGHT_1 + seq, 1.0, TOTAL_STEPS, TOTAL_STEPS));
	addInput(createInput<PJ301MPort>(Vec(x + 10, y + 14), module, Spiralone::INLENGHT_1 + seq));

	x += 60;
	addParam(createParam<BefacoSnappedTinyKnob>(Vec(x - 10, y - 12), module, Spiralone::STRIDE_1 + seq, 1.0, 8.0, 1.0));
	addInput(createInput<PJ301MPort>(Vec(x + 10, y + 14), module, Spiralone::INSTRIDE_1 + seq));

	x += 60;
	addParam(createParam<BefacoSnappedTinyKnob>(Vec(x - 10, y - 12), module, Spiralone::XPOSE_1 + seq, 0.0, 5.0, 0.0));
	addInput(createInput<PJ301MPort>(Vec(x + 10, y + 14), module, Spiralone::INXPOSE_1 + seq));

	x += 55;
	addOutput(createOutput<PJ301MPort>(Vec(x, y - 11), module, Spiralone::CV_1 + seq));
	addOutput(createOutput<PJ301GPort>(Vec(x, y + 19), module, Spiralone::GATE_1 + seq));

}

ModuleLightWidget *SpiraloneWidget::createLed(int seq, Vec pos, Module *module, int firstLightId, bool big)
{
	ModuleLightWidget * rv = new ModuleLightWidget();
	if(big)
		rv->box.size = mm2px(Vec(3, 3));
	else
		rv->box.size = mm2px(Vec(2.176, 2.176));
	rv->box.pos = pos;
	rv->addBaseColor(color[seq]);
	rv->module = module;
	rv->firstLightId = firstLightId;
	//rv->bgColor = COLOR_BLACK_TRANSPARENT;
	return rv;
}
