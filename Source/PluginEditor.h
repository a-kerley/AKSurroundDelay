#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"
#include "SliderModule.h"
#include "SurroundStageView.h"
#include "ViewPresetSelector.h"
#include "ResizeHandle.h"
#include "TapPanel.h"
#include <memory>
#include <array>

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
    
    // Custom LookAndFeel
    CustomLookAndFeel customLookAndFeel;
    
    // Number of taps
    static constexpr int NUM_TAPS = 8;
    
    // Tap tab bar (selects which tap panel is visible)
    TapTabBar tapTabBar;
    
    // All 8 tap panels (only one visible at a time)
    std::array<std::unique_ptr<TapPanel>, NUM_TAPS> tapPanels;
    int currentTapIndex = 0;
    
    // 3D Surround Stage View
    SurroundStageView surroundStageView;
    
    // View preset selector (segmented control)
    ViewPresetSelector viewPresetSelector;
    
    // Resize handle for bottom-right corner
    ResizeHandle resizeHandle;
    
    // Current UI scale factor (1.0 to 3.0)
    float currentScaleFactor = 1.0f;
    
    void setupTapPanels();
    void showTapPanel (int index);
    void setupViewPresetSelector();
    void setupResizeHandle();
    void updateAllComponentScales();  // Update scale factor on all child components

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapMatrixAudioProcessorEditor)
};

