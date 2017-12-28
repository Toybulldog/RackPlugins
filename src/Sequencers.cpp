#include "Klee.hpp"
#include "M581.hpp"

// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p)
{
	plugin = p;
	// This is the unique identifier for your plugin
	p->slug = "TheXOR";
#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif
	p->website = "https://github.com/The-XOR/VCV-Sequencers";
	p->manual = "https://github.com/The-XOR/VCV-Sequencers/blob/master/README.md";

	// For each module, specify the ModuleWidget subclass, manufacturer slug (for saving in patches), manufacturer human-readable name, module slug, and module name
	p->addModel(createModel<KleeWidget>("TheXOR", "Klee", "Klee Sequencer", SEQUENCER_TAG));
    p->addModel(createModel<M581Widget>("TheXOR", "M581", "581 Sequencer", SEQUENCER_TAG));

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
