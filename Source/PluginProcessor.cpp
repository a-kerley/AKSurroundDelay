#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SurroundDelayAudioProcessor::SurroundDelayAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    // Don't fix the output layout - let isBusesLayoutSupported() handle it dynamically
}

SurroundDelayAudioProcessor::~SurroundDelayAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout SurroundDelayAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // Delay Time: 0-2000ms
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        PARAM_DELAY_TIME,
        "Delay Time",
        juce::NormalisableRange<float> (0.0f, 2000.0f, 0.1f, 0.5f),
        250.0f,
        "ms"
    ));
    
    // Feedback: 0-95%
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        PARAM_FEEDBACK,
        "Feedback",
        juce::NormalisableRange<float> (0.0f, 0.95f, 0.01f),
        0.3f,
        "%",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value * 100.0f, 1); }
    ));
    
    // Mix: 0-100%
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        PARAM_MIX,
        "Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        0.5f,
        "%",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value * 100.0f, 1); }
    ));
    
    return layout;
}

//==============================================================================
const juce::String SurroundDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SurroundDelayAudioProcessor::acceptsMidi() const
{
    return false;
}

bool SurroundDelayAudioProcessor::producesMidi() const
{
    return false;
}

bool SurroundDelayAudioProcessor::isMidiEffect() const
{
    return false;
}

double SurroundDelayAudioProcessor::getTailLengthSeconds() const
{
    const float delayTimeMs = parameters.getRawParameterValue (PARAM_DELAY_TIME)->load();
    const float feedback = parameters.getRawParameterValue (PARAM_FEEDBACK)->load();
    
    // Calculate tail based on delay time and feedback
    // Higher feedback = longer tail
    const double delaySeconds = delayTimeMs / 1000.0;
    const double feedbackMultiplier = 1.0 / (1.0 - juce::jmin (0.95, static_cast<double> (feedback)));
    
    return delaySeconds * feedbackMultiplier;
}

int SurroundDelayAudioProcessor::getNumPrograms()
{
    return 1;   // Some hosts don't handle 0 programs correctly
}

int SurroundDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SurroundDelayAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String SurroundDelayAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void SurroundDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void SurroundDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);
    
    // Allocate delay buffer for maximum delay time (2 seconds) + safety margin
    const int maxDelaySamples = static_cast<int> (sampleRate * 2.5);
    const int numChannels = juce::jmax (getTotalNumInputChannels(), getTotalNumOutputChannels());
    
    delayBufferLength = maxDelaySamples;
    delayBuffer.setSize (numChannels, delayBufferLength);
    delayBuffer.clear();
    
    writePosition = 0;
}

void SurroundDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool SurroundDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    auto inputLayout  = layouts.getMainInputChannelSet();
    auto outputLayout = layouts.getMainOutputChannelSet();
    
    const int numIns  = inputLayout.size();
    const int numOuts = outputLayout.size();
    
    // Support pass-through configurations (same in/out)
    if (numIns == numOuts)
    {
        // Support up to 8 channels (7.1)
        if (numIns >= 1 && numIns <= 8)
            return true;
    }
    
    // Support upmixing from smaller inputs to larger outputs
    // Mono input can go to any output
    if (numIns == 1 && numOuts >= 1 && numOuts <= 8)
        return true;
    
    // Stereo input can go to stereo, 5.1, or 7.1
    if (numIns == 2)
    {
        if (numOuts == 2 || numOuts == 6 || numOuts == 8)
            return true;
    }
    
    return false;
}

void SurroundDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                 juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();
    
    // Get parameter values
    const float delayTimeSamples = getDelayTimeSamples();
    const float feedback = parameters.getRawParameterValue (PARAM_FEEDBACK)->load();
    const float mix = parameters.getRawParameterValue (PARAM_MIX)->load();
    
    // Process each input channel
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        auto* delayData = delayBuffer.getWritePointer (channel);
        
        int localWritePosition = writePosition;
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            const float inputSample = channelData[sample];
            
            // Calculate read position with fractional delay
            float readPosition = localWritePosition - delayTimeSamples;
            if (readPosition < 0)
                readPosition += delayBufferLength;
            
            // Linear interpolation for smooth delay
            int readIndex1 = static_cast<int> (readPosition);
            int readIndex2 = (readIndex1 + 1) % delayBufferLength;
            float frac = readPosition - readIndex1;
            
            float delayedSample = delayData[readIndex1] * (1.0f - frac) +
                                  delayData[readIndex2] * frac;
            
            // Write to delay buffer with feedback
            delayData[localWritePosition] = inputSample + (delayedSample * feedback);
            
            // Mix dry and wet signals
            channelData[sample] = inputSample * (1.0f - mix) + delayedSample * mix;
            
            // Advance write position
            localWritePosition = (localWritePosition + 1) % delayBufferLength;
        }
    }
    
    // Update write position
    writePosition = (writePosition + numSamples) % delayBufferLength;
    
    // Handle upmixing for surround outputs
    if (totalNumOutputChannels > totalNumInputChannels)
    {
        // Copy stereo to surround channels (simple upmix)
        if (totalNumInputChannels == 2)
        {
            // For 5.1: L/R to front L/R, copy to surround L/R, clear center and LFE
            if (totalNumOutputChannels >= 6)
            {
                // Surround Left (channel 4)
                if (totalNumOutputChannels > 4)
                {
                    buffer.copyFrom (4, 0, buffer, 0, 0, numSamples);
                    buffer.applyGain (4, 0, numSamples, 0.7f);
                }
                
                // Surround Right (channel 5)
                if (totalNumOutputChannels > 5)
                {
                    buffer.copyFrom (5, 0, buffer, 1, 0, numSamples);
                    buffer.applyGain (5, 0, numSamples, 0.7f);
                }
                
                // Center (channel 2) - mix L+R
                if (totalNumOutputChannels > 2)
                {
                    buffer.clear (2, 0, numSamples);
                    buffer.addFrom (2, 0, buffer, 0, 0, numSamples, 0.5f);
                    buffer.addFrom (2, 0, buffer, 1, 0, numSamples, 0.5f);
                }
                
                // LFE (channel 3) - low-passed mix (simplified: just attenuated mix)
                if (totalNumOutputChannels > 3)
                {
                    buffer.clear (3, 0, numSamples);
                    buffer.addFrom (3, 0, buffer, 0, 0, numSamples, 0.3f);
                    buffer.addFrom (3, 0, buffer, 1, 0, numSamples, 0.3f);
                }
                
                // For 7.1: also handle back surround channels (6 and 7)
                if (totalNumOutputChannels >= 8)
                {
                    buffer.copyFrom (6, 0, buffer, 4, 0, numSamples);
                    buffer.applyGain (6, 0, numSamples, 0.8f);
                    buffer.copyFrom (7, 0, buffer, 5, 0, numSamples);
                    buffer.applyGain (7, 0, numSamples, 0.8f);
                }
            }
        }
        else if (totalNumInputChannels == 1)
        {
            // Mono to surround: copy to all channels with appropriate levels
            for (int channel = 1; channel < totalNumOutputChannels; ++channel)
            {
                float level = (channel == 3) ? 0.5f : 1.0f; // Reduce LFE level
                buffer.copyFrom (channel, 0, buffer, 0, 0, numSamples);
                buffer.applyGain (channel, 0, numSamples, level);
            }
        }
    }
}

//==============================================================================
bool SurroundDelayAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* SurroundDelayAudioProcessor::createEditor()
{
    return new SurroundDelayAudioProcessorEditor (*this);
}

//==============================================================================
float SurroundDelayAudioProcessor::getDelayTimeSamples() const
{
    const float delayTimeMs = parameters.getRawParameterValue (PARAM_DELAY_TIME)->load();
    const float delaySamples = (delayTimeMs / 1000.0f) * static_cast<float> (getSampleRate());
    return juce::jlimit (0.0f, static_cast<float> (delayBufferLength - 1), delaySamples);
}

void SurroundDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void SurroundDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SurroundDelayAudioProcessor();
}
