// One-time utility to generate color variant cache files
// Compile and run this once when you change the color palette

#include "Source/SliderModule.h"
#include <juce_gui_basics/juce_gui_basics.h>

int main()
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    
    std::cout << "Generating color variant cache..." << std::endl;
    SliderModule::generateColorVariantCache();
    std::cout << "Done! Check assets/ folder for SliderFill_color0.png through SliderFill_color9.png" << std::endl;
    
    return 0;
}
