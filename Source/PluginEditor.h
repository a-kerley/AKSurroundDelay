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
    
    // Slider modules
    SliderModule mixSlider {"MIX"};
    SliderModule hueSlider {"HUE"};
    
    // 3D Surround Stage View
    SurroundStageView surroundStageView;
    
    // View preset selector (segmented control)
    ViewPresetSelector viewPresetSelector;
    
    void setupViewPresetSelector();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapMatrixAudioProcessorEditor)
};

