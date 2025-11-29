#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SurroundDelayAudioProcessor::SurroundDelayAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                      )
{
}

SurroundDelayAudioProcessor::~SurroundDelayAudioProcessor()
{
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
    return 0.0;
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
    // Use this method as the place to do any pre-playback
    // initialisation that you need.
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void SurroundDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool SurroundDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Currently supporting stereo only
    // Input and output must both be stereo
    if (layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo()
     && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo())
        return true;

    return false;
}

void SurroundDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                 juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is a simple pass-through
    // The input is already in the buffer, so we don't need to do anything
    // In the future, this is where the delay processing will happen
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
void SurroundDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void SurroundDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SurroundDelayAudioProcessor();
}
