#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * SurroundDelay Audio Processor Editor
 * 
 * This is the GUI component for the Surround Delay plugin.
 * Currently displays a minimal interface with a text label.
 */
class SurroundDelayAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    SurroundDelayAudioProcessorEditor (SurroundDelayAudioProcessor&);
    ~SurroundDelayAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for the editor to
    // access the processor object that created it.
    SurroundDelayAudioProcessor& audioProcessor;

    juce::Label titleLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SurroundDelayAudioProcessorEditor)
};
