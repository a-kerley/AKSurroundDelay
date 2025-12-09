#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"
#include "SliderModule.h"
#include "SurroundStageView.h"
#include "ViewPresetSelector.h"
#include "ResizeHandle.h"
#include <memory>

//==============================================================================
/**
 * TapMatrix Audio Processor Editor
 * 
 * Supports UI scaling from 1.0x to 3.0x with locked 55:41 aspect ratio.
 * Base size is 1100x820 pixels. Scale factor is stepped by 0.1.
 * All child components should call getScaleFactor() to scale their dimensions.
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
    
    //==========================================================================
    // UI SCALING
    //==========================================================================
    /** Get the current UI scale factor (1.0 to 3.0, stepped by 0.1) */
    float getScaleFactor() const { return currentScaleFactor; }
    
    /** Set the UI scale factor and resize the window accordingly */
    void setUIScaleFactorAndResize (float newScale);

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
    
    // Resize handle for bottom-right corner
    ResizeHandle resizeHandle;
    
    // Current UI scale factor (1.0 to 3.0)
    float currentScaleFactor = 1.0f;
    
    void setupViewPresetSelector();
    void setupResizeHandle();
    void updateAllComponentScales();  // Update scale factor on all child components

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapMatrixAudioProcessorEditor)
};

