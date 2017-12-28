#include "rack.hpp"
using namespace rack;
extern Plugin *plugin;

#include "dsp/digital.hpp"


#include <sstream>
#include <iomanip>


////////////////////
// module widgets
////////////////////

struct M581Widget : ModuleWidget
{
	M581Widget();
};

struct GateSwitch : SVGSwitch, ToggleSwitch
{
	GateSwitch()
	{
		addFrame(SVG::load(assetPlugin(plugin, "res/GateSwitch_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/GateSwitch_1.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/GateSwitch_2.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/GateSwitch_3.svg")));
	}
};


struct CounterSwitch : SVGSwitch, ToggleSwitch
{
	CounterSwitch()
	{
		addFrame(SVG::load(assetPlugin(plugin, "res/CounterSwitch_0.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CounterSwitch_1.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CounterSwitch_2.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CounterSwitch_3.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CounterSwitch_4.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CounterSwitch_5.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CounterSwitch_6.svg")));
		addFrame(SVG::load(assetPlugin(plugin, "res/CounterSwitch_7.svg")));
	}
};

struct SigDisplayWidget : TransparentWidget {

  int *value;
  std::shared_ptr<Font> font;

  SigDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));


  };

  void draw(NVGcontext *vg) override
  {
    // Background
    NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
    NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);
    nvgStrokeWidth(vg, 1.0);
    nvgStrokeColor(vg, borderColor);
    nvgStroke(vg);
    // text
    nvgFontSize(vg, 18);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);

    std::stringstream to_display;
    to_display << std::setw(2) << *value;

    Vec textPos = Vec(3, 17);

    NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "~~", NULL);

    textColor = nvgRGB(0xda, 0xe9, 0x29);
    nvgFillColor(vg, nvgTransRGBA(textColor, 16));
    nvgText(vg, textPos.x, textPos.y, "\\\\", NULL);

    textColor = nvgRGB(0xf0, 0x00, 0x00);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  }
};

struct RunModeDisplay : TransparentWidget
{
	int *mode;
	std::shared_ptr<Font> font;

	RunModeDisplay()
	{
		font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
	}

	void draw(NVGcontext *vg) override {
		 // Background
    NVGcolor backgroundColor = nvgRGB(0x20, 0x20, 0x20);
    NVGcolor borderColor = nvgRGB(0x10, 0x10, 0x10);
    nvgBeginPath(vg);
    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 4.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);
    nvgStrokeWidth(vg, 1.0);
    nvgStrokeColor(vg, borderColor);
    nvgStroke(vg);
    // text
    nvgFontSize(vg, 18);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);


		Vec textPos = Vec(2, 18);
        NVGcolor textColor = nvgRGB(0xdf, 0xd2, 0x2c);
        nvgFillColor(vg, nvgTransRGBA(textColor, 16));
        nvgText(vg, textPos.x, textPos.y, "~~", NULL);

        textColor = nvgRGB(0xda, 0xe9, 0x29);
        nvgFillColor(vg, nvgTransRGBA(textColor, 16));
        nvgText(vg, textPos.x, textPos.y, "\\\\", NULL);

        textColor = nvgRGB(0xf0, 0x00, 0x00);
        nvgFillColor(vg, textColor);
        nvgText(vg, textPos.x, textPos.y, run_modes[*mode], NULL);
	}

private:
    const char *run_modes[5] = {
        "FWD",
        "BWD",
        "PNG",
        "BRN",
        "RND"
    };
};
