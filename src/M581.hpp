#include "rack.hpp"
#include "common.hpp"

#include "dsp/digital.hpp"


#include <sstream>
#include <iomanip>

#include <algorithm>

////////////////////
// module widgets
////////////////////

struct M581Widget : ModuleWidget
{
private:
    enum MENUACTIONS
    {
        RANDOMIZE_PITCH,
        RANDOMIZE_COUNTER,
        RANDOMIZE_MODE,
        RANDOMIZE_ENABLE
    };

    struct KleeMenuItem : MenuItem
    {
        KleeMenuItem(const char *title, M581Widget *pW, MENUACTIONS act)
        {
            text = title;
            widget = pW;
            action = act;
        };

        void onAction(EventAction &e) override {widget->onMenu(action);};

        private:
            M581Widget *widget;
            MENUACTIONS action;
    };

    int getParamIndex(int index)
    {
        auto it = std::find_if(params.begin(), params.end(), [&index](const ParamWidget *m) -> bool { return m->paramId == index; });
        if(it != params.end())
            return std::distance(params.begin(), it);

        return -1;
    }

    void std_randomize(int first_index)
    {
        for (int k = 0; k < 8; k++)
        {
            int index = getParamIndex(first_index + k);
            if(index >= 0)
                params[index]->randomize();
        }
    }

public:
   	M581Widget();
    Menu *createContextMenu() override;
    void onMenu(MENUACTIONS action);

};

struct GateSwitch : SVGSlider
{
	GateSwitch()
	{
		snap = true;
		maxHandlePos = Vec(-4, 0);
		minHandlePos = Vec(-4, 37);
		background->svg = SVG::load(assetPlugin(plugin,"res/counterSwitchShort.svg"));
		background->wrap();
		background->box.pos = Vec(0, 0);
		box.size = background->box.size;
		handle->svg = SVG::load(assetPlugin(plugin,"res/counterSwitchPotHandle.svg"));
		handle->wrap();
	}

    void randomize() override {setValue(roundf(randomf() * maxValue));  }

};


struct CounterSwitch : SVGSlider
{
	CounterSwitch()
	{
		snap = true;
		maxHandlePos = Vec(-4, 0);
		minHandlePos = Vec(-4, 80);
		background->svg = SVG::load(assetPlugin(plugin,"res/counterSwitchPot.svg"));
		background->wrap();
		background->box.pos = Vec(0, 0);
		box.size = background->box.size;
		handle->svg = SVG::load(assetPlugin(plugin,"res/counterSwitchPotHandle.svg"));
		handle->wrap();
	}

	void randomize() override {setValue(roundf(randomf() * maxValue));  }
};

struct BefacoSnappedTinyKnob : BefacoTinyKnob
{
    BefacoSnappedTinyKnob() : BefacoTinyKnob()
    {
        snap = true;
    }
    void randomize() override {setValue(roundf(rescalef(randomf(), 0.0, 1.0, minValue, maxValue))); }
};

struct SigDisplayWidget : TransparentWidget {

  float *value;
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
    to_display << std::setw(2) << std::round(*value);

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
	float *mode;
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
    nvgText(vg, textPos.x, textPos.y, run_modes[int(std::round(*mode))], NULL);
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
