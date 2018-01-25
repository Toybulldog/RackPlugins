#pragma once
#define LVL_ON    (10.0)
#define LVL_OFF   (0.0)

using namespace rack;
extern Plugin *plugin;

struct PJ301YPort : SVGPort {
	PJ301YPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/PJ301Y.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct PJ301GPort : SVGPort {
	PJ301GPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/PJ301G.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct PJ301RPort : SVGPort {
	PJ301RPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/PJ301R.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};

struct PJ301WPort : SVGPort {
	PJ301WPort() {
		background->svg = SVG::load(assetPlugin(plugin, "res/PJ301W.svg"));
		background->wrap();
		box.size = background->box.size;
	}
};
