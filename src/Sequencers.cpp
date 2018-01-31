#include "Klee.hpp"
#include "M581.hpp"
#include "Z8K.hpp"
#include "Renato.hpp"
#ifdef TEST_MODULE
#include "lpTestModule.hpp"
#endif // defined

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
	p->addModel(createModel<Z8KWidget>("TheXOR", "Z8K", "Z8K Sequencer", SEQUENCER_TAG));
	p->addModel(createModel<RenatoWidget>("TheXOR", "Renato", "Renato Sequencer", SEQUENCER_TAG));

	#ifdef TEST_MODULE
	p->addModel(createModel<LaunchpadTestWidget>("TheXOR", "LaunchpadTest", "Launchpad Test", DIGITAL_TAG));
	#endif


	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
