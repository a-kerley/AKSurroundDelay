#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
/**
 * SurroundDelay Audio Processor
 * 
 * This is the main audio processing class for the Surround Delay plugin.
 * Currently implements stereo pass-through processing.
 */
class SurroundDelayAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    SurroundDelayAudioProcessor();
    ~SurroundDelayAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==============================================================================
    // Public access to parameters for editor
    juce::AudioProcessorValueTreeState& getParameters() { return parameters; }

private:
    //==============================================================================
    // Parameter tree state for automation and preset management
    juce::AudioProcessorValueTreeState parameters;
    
    // Parameter IDs
    static constexpr const char* PARAM_DELAY_TIME = "delayTime";
    static constexpr const char* PARAM_FEEDBACK = "feedback";
    static constexpr const char* PARAM_MIX = "mix";
    
    // Delay buffers (one per channel, up to 8 channels for 7.1)
    juce::AudioBuffer<float> delayBuffer;
    int delayBufferLength = 0;
    int writePosition = 0;
    
    // Helper functions
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    float getDelayTimeSamples() const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SurroundDelayAudioProcessor)
};
