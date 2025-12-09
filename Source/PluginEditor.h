#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"
#include "SliderModule.h"
#include "SurroundStageView.h"
#include "ViewPresetSelector.h"
#include <memory>

//==============================================================================
/**
 * TapMatrix Audio Processor Editor - Clean Slate
 * 
 * Ready for custom UI implementation
 */
class TapMatrixAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       public juce::Timer
{
public:
    TapMatrixAudioProcessorEditor (TapMatrixAudioProcessor&);
    ~TapMatrixAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    // Timer callback to sync view preset state
    void timerCallback() override;

private:
    // Reference to processor
    TapMatrixAudioProcessor& audioProcessor;
    
    // Update slider colors based on hue parameter
    void updateSliderColors();
    
    // Custom LookAndFeel
    CustomLookAndFeel customLookAndFeel;
    
    // Slider modules - one of each style for testing
    SliderModule slider_38x170 {"38x170", FaderStyle::Fader_38x170};
    SliderModule slider_22x170 {"22x170", FaderStyle::Fader_22x170};
    SliderModule slider_32x129 {"32x129", FaderStyle::Fader_32x129};
    SliderModule slider_32x129FB {"32x129FB", FaderStyle::Fader_32x129_FrontBack};
    SliderModule slider_22x79 {"22x79", FaderStyle::Fader_22x79};
    SliderModule slider_28x84H {"28x84H", FaderStyle::Fader_28x84_HorizontalLeftRight};
    
    // Hue slider for color testing
    SliderModule hueSlider {"HUE", FaderStyle::Fader_38x170};
    
    // 3D Surround Stage View
    SurroundStageView surroundStageView;
    
    // View preset selector (segmented control)
    ViewPresetSelector viewPresetSelector;
    
    void setupViewPresetSelector();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapMatrixAudioProcessorEditor)
};

