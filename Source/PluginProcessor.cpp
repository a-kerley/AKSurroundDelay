#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TapMatrixAudioProcessor::TapMatrixAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    // Initialize crosstalk matrix to zero
    for (int i = 0; i < NUM_TAPS; ++i)
        for (int j = 0; j < NUM_TAPS; ++j)
            crosstalkMatrix[i][j] = 0.0f;
}

TapMatrixAudioProcessor::~TapMatrixAudioProcessor()
{
}

juce::String TapMatrixAudioProcessor::getTapParamID (const char* paramName, int tapIndex)
{
    return juce::String (paramName) + juce::String (tapIndex + 1);
}

juce::AudioProcessorValueTreeState::ParameterLayout TapMatrixAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // Create parameters for each of the 8 taps
    for (int i = 0; i < NUM_TAPS; ++i)
    {
        juce::String tapName = "Tap " + juce::String (i + 1);
        
        // Gain: -∞ to 0 dB
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            getTapParamID ("gain", i),
            tapName + " Gain",
            juce::NormalisableRange<float> (-96.0f, 0.0f, 0.1f),
            0.0f,
            "dB"
        ));
        
        // Delay Time: 0-2500ms (TIME mode)
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            getTapParamID ("delayTime", i),
            tapName + " Delay",
            juce::NormalisableRange<float> (0.0f, MAX_DELAY_MS, 0.1f, 0.5f),
            100.0f * (i + 1),  // Spread taps out initially
            "ms"
        ));
        
        // Feedback: 0-100%
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            getTapParamID ("feedback", i),
            tapName + " Feedback",
            juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
            0.0f,
            "%",
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String (value * 100.0f, 1); }
        ));
        
        // Crosstalk: 0-100%
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            getTapParamID ("crosstalk", i),
            tapName + " Crosstalk",
            juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
            0.0f,
            "%",
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String (value * 100.0f, 1); }
        ));
        
        // Damping: 0-100%
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            getTapParamID ("damping", i),
            tapName + " Damping",
            juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
            0.0f,
            "%",
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String (value * 100.0f, 1); }
        ));
        
        // Reverb: 0-100%
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            getTapParamID ("reverb", i),
            tapName + " Reverb",
            juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
            0.0f,
            "%",
            juce::AudioProcessorParameter::genericParameter,
            [](float value, int) { return juce::String (value * 100.0f, 1); }
        ));
        
        // Pan X (L/R): -100 to +100
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            getTapParamID ("panX", i),
            tapName + " Pan X",
            juce::NormalisableRange<float> (-1.0f, 1.0f, 0.01f),
            0.0f,
            ""
        ));
        
        // Pan Y (F/B): -100 to +100
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            getTapParamID ("panY", i),
            tapName + " Pan Y",
            juce::NormalisableRange<float> (-1.0f, 1.0f, 0.01f),
            0.0f,
            ""
        ));
        
        // Pan Z (height): 0-100%
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            getTapParamID ("panZ", i),
            tapName + " Pan Z",
            juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
            0.0f,
            "%"
        ));
        
        // SYNC mode toggle (false = TIME in ms, true = SYNC in beats)
        layout.add (std::make_unique<juce::AudioParameterBool> (
            getTapParamID ("syncMode", i),
            tapName + " Sync Mode",
            false  // Default to TIME mode
        ));
        
        // Sync Delay (0-10 quarter notes, only used when syncMode = true)
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            getTapParamID ("syncDelay", i),
            tapName + " Sync Delay",
            juce::NormalisableRange<float> (0.0f, 10.0f, 0.01f, 0.5f),
            0.25f * (i + 1),  // Spread taps: 0.25, 0.5, 0.75... quarter notes
            "beats"
        ));
    }
    
    // Global parameters
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "mix",
        "Mix",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
        1.0f,
        "%",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String (value * 100.0f, 1); }
    ));
    
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "outputGain",
        "Output Gain",
        juce::NormalisableRange<float> (-96.0f, 6.0f, 0.1f),
        0.0f,
        "dB"
    ));
    
    // Debug/Testing: Hue control (0-9 for 10 palette colors)
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "hue",
        "Hue",
        juce::NormalisableRange<float> (0.0f, 9.0f, 1.0f),
        4.0f,  // Default to middle color
        ""
    ));
    
    // Global Reverb Type
    layout.add (std::make_unique<juce::AudioParameterChoice> (
        "reverbType",
        "Reverb Type",
        juce::StringArray { "Dark", "Short", "Medium", "Long", "XXXL" },
        2  // Default to Medium
    ));
    
    // Global HPF (20Hz - 20kHz)
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "hpfFreq",
        "HPF Frequency",
        juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.3f),  // Skewed towards lower frequencies
        20.0f,
        "Hz"
    ));
    
    // Global LPF (20Hz - 20kHz)
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "lpfFreq",
        "LPF Frequency",
        juce::NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.3f),
        20000.0f,
        "Hz"
    ));
    
    // Ducking (0-12 dB reduction)
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "ducking",
        "Ducking",
        juce::NormalisableRange<float> (0.0f, 12.0f, 0.1f),
        0.0f,
        "dB"
    ));
    
    // Tape Mode (smooth delay time changes)
    layout.add (std::make_unique<juce::AudioParameterBool> (
        "tapeMode",
        "Tape Mode",
        true  // Default ON for smooth, glitch-free delay changes
    ));
    
    return layout;
}

//==============================================================================
const juce::String TapMatrixAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TapMatrixAudioProcessor::acceptsMidi() const
{
    return false;
}

bool TapMatrixAudioProcessor::producesMidi() const
{
    return false;
}

bool TapMatrixAudioProcessor::isMidiEffect() const
{
    return false;
}

double TapMatrixAudioProcessor::getTailLengthSeconds() const
{
    // Calculate maximum tail length across all taps
    double maxTail = 0.0;
    
    for (int i = 0; i < NUM_TAPS; ++i)
    {
        auto delayTimeMs = parameters.getRawParameterValue (getTapParamID ("delayTime", i))->load();
        auto feedback = parameters.getRawParameterValue (getTapParamID ("feedback", i))->load();
        
        if (feedback > 0.0f && delayTimeMs > 0.0f)
        {
            double delaySeconds = delayTimeMs / 1000.0;
            double feedbackMultiplier = 1.0 / (1.0 - juce::jmin (0.99, static_cast<double> (feedback)));
            double tapTail = delaySeconds * feedbackMultiplier;
            maxTail = juce::jmax (maxTail, tapTail);
        }
    }
    
    return maxTail;
}

int TapMatrixAudioProcessor::getNumPrograms()
{
    return NUM_FACTORY_PRESETS;
}

int TapMatrixAudioProcessor::getCurrentProgram()
{
    return currentPresetIndex;
}

void TapMatrixAudioProcessor::setCurrentProgram (int index)
{
    if (index >= 0 && index < NUM_FACTORY_PRESETS)
    {
        currentPresetIndex = index;
        loadFactoryPreset (index);
    }
}

const juce::String TapMatrixAudioProcessor::getProgramName (int index)
{
    switch (index)
    {
        case 0: return "Init (Default)";
        case 1: return "Vintage Slap";
        case 2: return "Haas Widener";
        case 3: return "Rhythmic Bounce";
        case 4: return "Surround Wash";
        case 5: return "Ping Pong Delay";
        case 6: return "Tight Doubler";
        case 7: return "Spatial Echo";
        default: return "Unknown";
    }
}

void TapMatrixAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    // Factory presets cannot be renamed
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void TapMatrixAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Prepare all 8 delay taps
    for (auto& tap : taps)
        tap.prepareToPlay (sampleRate, MAX_DELAY_MS);
    
    // Prepare mono input buffer
    monoInputBuffer.setSize (1, samplesPerBlock);
    
    // Prepare tap output buffer (8 taps, mono each)
    tapOutputBuffer.setSize (NUM_TAPS, samplesPerBlock);
    
    // Prepare crosstalk buffer (pre-allocate to avoid real-time malloc)
    crosstalkBuffer.setSize (NUM_TAPS, samplesPerBlock);
    
    // Prepare dry buffer (max 8 channels for 7.1)
    dryBuffer.setSize (MAX_CHANNELS, samplesPerBlock);
    
    // Prepare reverb for each tap
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = 1;  // Mono reverb per tap
    
    for (auto& tap : taps)
        tap.reverb.prepare (spec);
    
    // Initialize reverb parameters based on current type
    updateReverbParameters();
    
    // Prepare global filters (HPF/LPF)
    juce::dsp::ProcessSpec filterSpec;
    filterSpec.sampleRate = sampleRate;
    filterSpec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    filterSpec.numChannels = 1;  // Process each channel independently
    
    for (int i = 0; i < MAX_CHANNELS; ++i)
    {
        hpFilters[i].prepare (filterSpec);
        lpFilters[i].prepare (filterSpec);
        
        // Set filter types (12dB/oct = 2-pole)
        hpFilters[i].setType (juce::dsp::StateVariableTPTFilterType::highpass);
        lpFilters[i].setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    }
    
    // Reset ducking envelope
    duckingEnvelopeSq = 0.0f;
}

void TapMatrixAudioProcessor::releaseResources()
{
    for (auto& tap : taps)
        tap.reset();
}

bool TapMatrixAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void TapMatrixAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();
    
    // Get host tempo for tempo sync (with safety checks)
    double currentBPM = 120.0;  // Default fallback
    if (auto* playHead = getPlayHead())
    {
        if (auto posInfo = playHead->getPosition())
        {
            if (auto bpm = posInfo->getBpm())
                currentBPM = juce::jlimit (20.0, 999.0, *bpm);  // Clamp to sane range
        }
    }
    
    // Clear any output channels beyond input channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, numSamples);
    
    // Step 1: Save dry signal for later mixing
    dryBuffer.clear();
    for (int ch = 0; ch < juce::jmin (totalNumInputChannels, MAX_CHANNELS); ++ch)
        dryBuffer.copyFrom (ch, 0, buffer, ch, 0, numSamples);
    
    // Step 2: Sum input to mono
    monoInputBuffer.clear();
    float invNumInputs = 1.0f / juce::jmax (1, totalNumInputChannels);
    
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
        monoInputBuffer.addFrom (0, 0, buffer, ch, 0, numSamples, invNumInputs);
    
    // Step 3: Process all taps (delay + reverb) with tempo sync
    processTaps (monoInputBuffer.getReadPointer (0), numSamples, currentBPM);
    
    // Step 4: Apply crosstalk mixing
    applyCrosstalk (numSamples);
    
    // Step 5: Apply panning to create wet signal in output buffer
    buffer.clear();  // Clear before panning writes to it
    applyPanning (buffer, numSamples);
    
    // Step 6: Apply global HPF/LPF to wet signal
    applyGlobalFilters (buffer, numSamples);
    
    // Step 7: Apply ducking to wet signal based on dry input
    applyDucking (buffer, dryBuffer, numSamples);
    
    // Step 8: Mix dry and wet signals
    applyDryWetMix (buffer, dryBuffer, buffer, numSamples);
    
    // Step 9: Apply output gain
    float outputGainDb = parameters.getRawParameterValue ("outputGain")->load();
    float outputGain = juce::Decibels::decibelsToGain (outputGainDb);
    buffer.applyGain (outputGain);
}

void TapMatrixAudioProcessor::processTaps (const float* monoInput, int numSamples, double bpm)
{
    tapOutputBuffer.clear();
    
    // Check if reverb type has changed
    int reverbTypeIndex = parameters.getRawParameterValue ("reverbType")->load();
    auto newReverbType = static_cast<ReverbType> (reverbTypeIndex);
    if (newReverbType != currentReverbType)
    {
        currentReverbType = newReverbType;
        updateReverbParameters();
    }
    
    // Get tape mode setting
    bool tapeMode = parameters.getRawParameterValue ("tapeMode")->load() > 0.5f;
    
    for (int tapIndex = 0; tapIndex < NUM_TAPS; ++tapIndex)
    {
        auto& tap = taps[tapIndex];
        auto* tapOutput = tapOutputBuffer.getWritePointer (tapIndex);
        auto* delayData = tap.buffer.getWritePointer (0);
        
        // Get tap parameters
        float gainDb = parameters.getRawParameterValue (getTapParamID ("gain", tapIndex))->load();
        float gain = juce::Decibels::decibelsToGain (gainDb);
        
        // Check sync mode
        bool useSyncMode = parameters.getRawParameterValue (getTapParamID ("syncMode", tapIndex))->load() > 0.5f;
        float delayTimeMs;
        
        if (useSyncMode)
        {
            // SYNC mode: convert beats to milliseconds
            float syncDelayBeats = parameters.getRawParameterValue (getTapParamID ("syncDelay", tapIndex))->load();
            delayTimeMs = beatsToMs (syncDelayBeats, bpm);
        }
        else
        {
            // TIME mode: use direct millisecond value
            delayTimeMs = parameters.getRawParameterValue (getTapParamID ("delayTime", tapIndex))->load();
        }
        
        float feedback = parameters.getRawParameterValue (getTapParamID ("feedback", tapIndex))->load();
        feedback = juce::jlimit (0.0f, 0.995f, feedback);  // Hard limit to prevent runaway
        float damping = parameters.getRawParameterValue (getTapParamID ("damping", tapIndex))->load();
        float reverbAmount = parameters.getRawParameterValue (getTapParamID ("reverb", tapIndex))->load();
        
        // Convert delay time to samples
        tap.targetDelaySamples = (delayTimeMs / 1000.0f) * static_cast<float> (getSampleRate());
        tap.targetDelaySamples = juce::jlimit (1.0f, static_cast<float> (tap.bufferLength - 4), tap.targetDelaySamples);
        
        // Initialize current delay on first run
        if (tap.currentDelaySamples == 0.0f)
            tap.currentDelaySamples = tap.targetDelaySamples;
        
        // Update damping coefficient (1.0 = no damping, 0.0 = full damping)
        tap.dampingCoeff = 1.0f - damping;
        
        int localWritePos = tap.writePosition;
        
        for (int i = 0; i < numSamples; ++i)
        {
            // Tape mode: smoothly interpolate delay time to avoid clicks/glitches
            if (tapeMode)
            {
                // Smooth delay time changes over approximately 10ms (correct exponential formula)
                float smoothingCoeff = 1.0f - std::exp (-1.0f / (0.010f * static_cast<float> (getSampleRate())));
                tap.currentDelaySamples += smoothingCoeff * (tap.targetDelaySamples - tap.currentDelaySamples);
            }
            else
            {
                // No tape mode: instant delay time changes (can cause clicks)
                tap.currentDelaySamples = tap.targetDelaySamples;
            }
            
            // Calculate read position using current smoothed delay time
            float readPos = localWritePos - tap.currentDelaySamples;
            if (readPos < 0.0f)
                readPos += tap.bufferLength;
            
            // Cubic interpolation for higher quality (reduces aliasing artifacts)
            int readIndex0 = (static_cast<int> (readPos) - 1 + tap.bufferLength) % tap.bufferLength;
            int readIndex1 = static_cast<int> (readPos);
            int readIndex2 = (readIndex1 + 1) % tap.bufferLength;
            int readIndex3 = (readIndex1 + 2) % tap.bufferLength;
            float frac = readPos - std::floor (readPos);
            
            float y0 = delayData[readIndex0];
            float y1 = delayData[readIndex1];
            float y2 = delayData[readIndex2];
            float y3 = delayData[readIndex3];
            
            float delayedSample = cubicInterpolate (y0, y1, y2, y3, frac);
            
            // Apply damping to feedback (feedback does NOT include reverb)
            float dampedFeedback = delayedSample * tap.dampingCoeff;
            
            // Write to delay buffer with feedback (with safety clipping and denormal flush)
            float newSample = monoInput[i] + (dampedFeedback * feedback);
            newSample = juce::jlimit (-1.5f, 1.5f, newSample);  // Prevent runaway
            // Flush denormals to zero for CPU efficiency
            if (std::fpclassify (newSample) == FP_SUBNORMAL)
                newSample = 0.0f;
            delayData[localWritePos] = newSample;
            
            // Output delayed sample with gain (dry delay signal)
            tapOutput[i] = delayedSample * gain;
            
            // Advance write position
            localWritePos = (localWritePos + 1) % tap.bufferLength;
        }
        
        tap.writePosition = localWritePos;
        
        // Apply reverb to tap output if reverb amount > 0
        // Reverb is applied POST-delay, PRE-panning
        // The spec says: "Reverb is not included in feedback or crosstalk"
        if (reverbAmount > 0.001f)
        {
            // Create a copy of the dry delay signal for reverb processing
            juce::AudioBuffer<float> reverbBuffer (1, numSamples);
            reverbBuffer.copyFrom (0, 0, tapOutput, numSamples);
            
            // Process reverb on the copy
            juce::dsp::AudioBlock<float> block (reverbBuffer);
            juce::dsp::ProcessContextReplacing<float> context (block);
            tap.reverb.process (context);
            
            // Blend dry delay with wet reverb
            // tapOutput = (1 - reverbAmount) * dry + reverbAmount * wet
            float dryGain = 1.0f - reverbAmount;
            
            for (int i = 0; i < numSamples; ++i)
                tapOutput[i] = tapOutput[i] * dryGain + reverbBuffer.getSample (0, i) * reverbAmount;
        }
        
        // Calculate RMS level for UI metering (pre-pan)
        float sumSquares = 0.0f;
        for (int i = 0; i < numSamples; ++i)
            sumSquares += tapOutput[i] * tapOutput[i];
        float rms = std::sqrt (sumSquares / numSamples);
        
        // Smooth the level with decay for visual appeal
        const float attackCoeff = 0.8f;
        const float releaseCoeff = 0.95f;
        float currentVal = tap.currentLevel.load();
        float newVal = (rms > currentVal) ? (currentVal * attackCoeff + rms * (1.0f - attackCoeff))
                                           : (currentVal * releaseCoeff);
        tap.currentLevel.store (newVal);
        
        tap.lastOutputSample = tapOutput[numSamples - 1];
    }
}

void TapMatrixAudioProcessor::applyCrosstalk (int numSamples)
{
    // Use pre-allocated member buffer (no malloc on audio thread)
    crosstalkBuffer.clear();
    
    // Calculate crosstalk for each tap
    for (int destTap = 0; destTap < NUM_TAPS; ++destTap)
    {
        auto* destBuffer = crosstalkBuffer.getWritePointer (destTap);
        
        for (int srcTap = 0; srcTap < NUM_TAPS; ++srcTap)
        {
            if (srcTap == destTap)
                continue;  // No self-crosstalk
            
            float crosstalkAmount = parameters.getRawParameterValue (getTapParamID ("crosstalk", srcTap))->load();
            
            if (crosstalkAmount > 0.0f)
            {
                auto* srcBuffer = tapOutputBuffer.getReadPointer (srcTap);
                
                for (int i = 0; i < numSamples; ++i)
                    destBuffer[i] += srcBuffer[i] * crosstalkAmount;
            }
        }
    }
    
    // Add crosstalk back to tap outputs
    for (int tap = 0; tap < NUM_TAPS; ++tap)
        tapOutputBuffer.addFrom (tap, 0, crosstalkBuffer, tap, 0, numSamples);
}

void TapMatrixAudioProcessor::applyPanning (juce::AudioBuffer<float>& outputBuffer, int numSamples)
{
    outputBuffer.clear();
    
    const int numOutputChannels = outputBuffer.getNumChannels();
    
    for (int tapIndex = 0; tapIndex < NUM_TAPS; ++tapIndex)
    {
        // Route based on output channel count
        if (numOutputChannels == 1)
        {
            // Mono output - sum all taps to center
            auto* tapInput = tapOutputBuffer.getReadPointer (tapIndex);
            outputBuffer.addFrom (0, 0, tapInput, numSamples);
        }
        else if (numOutputChannels == 2)
        {
            // Stereo output - use X panning
            panTapToStereo (tapIndex, outputBuffer.getWritePointer (0), 
                           outputBuffer.getWritePointer (1), numSamples);
        }
        else if (numOutputChannels == 6)
        {
            // 5.1 surround
            panTapTo51 (tapIndex, outputBuffer, numSamples);
        }
        else if (numOutputChannels == 8)
        {
            // 7.1 surround
            panTapTo71 (tapIndex, outputBuffer, numSamples);
        }
        else
        {
            // Multi-mono or other formats - distribute evenly
            auto* tapInput = tapOutputBuffer.getReadPointer (tapIndex);
            for (int ch = 0; ch < numOutputChannels; ++ch)
                outputBuffer.addFrom (ch, 0, tapInput, numSamples, 1.0f / numOutputChannels);
        }
    }
}

void TapMatrixAudioProcessor::panTapToStereo (int tapIndex, float* leftOut, float* rightOut, int numSamples)
{
    auto* tapInput = tapOutputBuffer.getReadPointer (tapIndex);
    float panX = parameters.getRawParameterValue (getTapParamID ("panX", tapIndex))->load();
    
    // Constant power pan law: L = cos(θ), R = sin(θ)
    // panX ranges from -1 (left) to +1 (right)
    float panAngle = (panX + 1.0f) * 0.25f * juce::MathConstants<float>::pi; // 0 to π/2
    float leftGain = std::cos (panAngle);
    float rightGain = std::sin (panAngle);
    
    for (int i = 0; i < numSamples; ++i)
    {
        leftOut[i] += tapInput[i] * leftGain;
        rightOut[i] += tapInput[i] * rightGain;
    }
}

void TapMatrixAudioProcessor::panTapTo51 (int tapIndex, juce::AudioBuffer<float>& outputBuffer, int numSamples)
{
    auto* tapInput = tapOutputBuffer.getReadPointer (tapIndex);
    
    // Get pan coordinates
    float panX = parameters.getRawParameterValue (getTapParamID ("panX", tapIndex))->load();  // -1 to +1 (L/R)
    float panY = parameters.getRawParameterValue (getTapParamID ("panY", tapIndex))->load();  // -1 to +1 (F/B)
    
    // 5.1 speaker layout: L(0), R(1), C(2), LFE(3), Ls(4), Rs(5)
    // Normalize X,Y to 0-1 range
    float x = (panX + 1.0f) * 0.5f; // 0 = left, 1 = right
    float y = (panY + 1.0f) * 0.5f; // 0 = front, 1 = back
    
    // Calculate gains for each speaker
    float gainL = 0.0f, gainR = 0.0f, gainC = 0.0f, gainLs = 0.0f, gainRs = 0.0f;
    
    if (y < 0.5f)
    {
        // Front hemisphere
        float frontAmount = 1.0f - (y * 2.0f);
        
        if (x < 0.33f)
        {
            // Left front
            gainL = frontAmount * (1.0f - (x * 3.0f));
            gainC = frontAmount * (x * 3.0f);
        }
        else if (x < 0.66f)
        {
            // Center front
            float centerX = (x - 0.33f) * 3.0f;
            gainC = frontAmount * (1.0f - centerX);
            gainR = frontAmount * centerX;
        }
        else
        {
            // Right front
            gainC = frontAmount * (1.0f - ((x - 0.66f) * 3.0f));
            gainR = frontAmount * ((x - 0.66f) * 3.0f);
        }
    }
    
    // Surround speakers (back hemisphere)
    float backAmount = y;
    gainLs = backAmount * (1.0f - x);
    gainRs = backAmount * x;
    
    // Apply gains
    for (int i = 0; i < numSamples; ++i)
    {
        float sample = tapInput[i];
        outputBuffer.getWritePointer(0)[i] += sample * gainL;   // L
        outputBuffer.getWritePointer(1)[i] += sample * gainR;   // R
        outputBuffer.getWritePointer(2)[i] += sample * gainC;   // C
        // Skip LFE (3)
        outputBuffer.getWritePointer(4)[i] += sample * gainLs;  // Ls
        outputBuffer.getWritePointer(5)[i] += sample * gainRs;  // Rs
    }
}

void TapMatrixAudioProcessor::panTapTo71 (int tapIndex, juce::AudioBuffer<float>& outputBuffer, int numSamples)
{
    auto* tapInput = tapOutputBuffer.getReadPointer (tapIndex);
    
    // Get pan coordinates
    float panX = parameters.getRawParameterValue (getTapParamID ("panX", tapIndex))->load();
    float panY = parameters.getRawParameterValue (getTapParamID ("panY", tapIndex))->load();
    
    // 7.1 speaker layout: L(0), R(1), C(2), LFE(3), Ls(4), Rs(5), Lrs(6), Rrs(7)
    float x = (panX + 1.0f) * 0.5f;
    float y = (panY + 1.0f) * 0.5f;
    
    float gainL = 0.0f, gainR = 0.0f, gainC = 0.0f;
    float gainLs = 0.0f, gainRs = 0.0f, gainLrs = 0.0f, gainRrs = 0.0f;
    
    // Front speakers (similar to 5.1)
    if (y < 0.5f)
    {
        float frontAmount = 1.0f - (y * 2.0f);
        
        if (x < 0.33f)
        {
            gainL = frontAmount * (1.0f - (x * 3.0f));
            gainC = frontAmount * (x * 3.0f);
        }
        else if (x < 0.66f)
        {
            float centerX = (x - 0.33f) * 3.0f;
            gainC = frontAmount * (1.0f - centerX);
            gainR = frontAmount * centerX;
        }
        else
        {
            gainC = frontAmount * (1.0f - ((x - 0.66f) * 3.0f));
            gainR = frontAmount * ((x - 0.66f) * 3.0f);
        }
    }
    
    // Surround speakers (split between side and rear)
    float backAmount = y;
    
    if (y < 0.75f)
    {
        // Side surrounds
        float sideAmount = backAmount * (1.0f - ((y - 0.5f) * 4.0f));
        gainLs = sideAmount * (1.0f - x);
        gainRs = sideAmount * x;
    }
    
    if (y > 0.5f)
    {
        // Rear surrounds
        float rearAmount = (y - 0.5f) * 2.0f;
        gainLrs = rearAmount * (1.0f - x);
        gainRrs = rearAmount * x;
    }
    
    // Apply gains
    for (int i = 0; i < numSamples; ++i)
    {
        float sample = tapInput[i];
        outputBuffer.getWritePointer(0)[i] += sample * gainL;    // L
        outputBuffer.getWritePointer(1)[i] += sample * gainR;    // R
        outputBuffer.getWritePointer(2)[i] += sample * gainC;    // C
        // Skip LFE (3)
        outputBuffer.getWritePointer(4)[i] += sample * gainLs;   // Ls (side left)
        outputBuffer.getWritePointer(5)[i] += sample * gainRs;   // Rs (side right)
        outputBuffer.getWritePointer(6)[i] += sample * gainLrs;  // Lrs (rear left)
        outputBuffer.getWritePointer(7)[i] += sample * gainRrs;  // Rrs (rear right)
    }
}

//==============================================================================
bool TapMatrixAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* TapMatrixAudioProcessor::createEditor()
{
    return new TapMatrixAudioProcessorEditor (*this);
}

//==============================================================================
void TapMatrixAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void TapMatrixAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// Reverb Configuration
//==============================================================================

void TapMatrixAudioProcessor::updateReverbParameters()
{
    auto params = getReverbPreset (currentReverbType);
    
    // Apply the same reverb parameters to all tap reverb instances
    for (auto& tap : taps)
        tap.reverb.setParameters (params);
}

juce::dsp::Reverb::Parameters TapMatrixAudioProcessor::getReverbPreset (ReverbType type) const
{
    juce::dsp::Reverb::Parameters params;
    
    switch (type)
    {
        case ReverbType::Dark:
            params.roomSize   = 0.5f;
            params.damping    = 0.8f;   // Heavy damping for dark character
            params.wetLevel   = 1.0f;   // 100% wet (we control mix via reverbAmount)
            params.dryLevel   = 0.0f;   // No dry signal from reverb
            params.width      = 1.0f;
            params.freezeMode = 0.0f;
            break;
            
        case ReverbType::Short:
            params.roomSize   = 0.3f;   // Small room
            params.damping    = 0.4f;   // Light damping
            params.wetLevel   = 1.0f;
            params.dryLevel   = 0.0f;
            params.width      = 0.8f;
            params.freezeMode = 0.0f;
            break;
            
        case ReverbType::Medium:
            params.roomSize   = 0.5f;   // Medium room
            params.damping    = 0.5f;   // Medium damping
            params.wetLevel   = 1.0f;
            params.dryLevel   = 0.0f;
            params.width      = 1.0f;
            params.freezeMode = 0.0f;
            break;
            
        case ReverbType::Long:
            params.roomSize   = 0.75f;  // Large room
            params.damping    = 0.3f;   // Less damping for longer tail
            params.wetLevel   = 1.0f;
            params.dryLevel   = 0.0f;
            params.width      = 1.0f;
            params.freezeMode = 0.0f;
            break;
            
        case ReverbType::XXXL:
            params.roomSize   = 0.95f;  // Huge space (film wash)
            params.damping    = 0.2f;   // Very light damping
            params.wetLevel   = 1.0f;
            params.dryLevel   = 0.0f;
            params.width      = 1.0f;
            params.freezeMode = 0.0f;
            break;
    }
    
    return params;
}

//==============================================================================
// Global Processing Chain
//==============================================================================

void TapMatrixAudioProcessor::applyGlobalFilters (juce::AudioBuffer<float>& buffer, int numSamples)
{
    const int numChannels = buffer.getNumChannels();
    
    // Get filter frequencies (clamp to Nyquist to prevent instability)
    float nyquist = static_cast<float> (getSampleRate()) * 0.49f;  // 2% headroom
    float hpfFreq = juce::jmin (parameters.getRawParameterValue ("hpfFreq")->load(), nyquist);
    float lpfFreq = juce::jmin (parameters.getRawParameterValue ("lpfFreq")->load(), nyquist);
    
    // Update filter cutoffs
    for (int ch = 0; ch < juce::jmin (numChannels, MAX_CHANNELS); ++ch)
    {
        hpFilters[ch].setCutoffFrequency (hpfFreq);
        lpFilters[ch].setCutoffFrequency (lpfFreq);
    }
    
    // Process each channel through HPF then LPF
    for (int ch = 0; ch < juce::jmin (numChannels, MAX_CHANNELS); ++ch)
    {
        auto* channelData = buffer.getWritePointer (ch);
        
        // Create audio block for this channel
        juce::dsp::AudioBlock<float> block (&channelData, 1, 0, numSamples);
        juce::dsp::ProcessContextReplacing<float> context (block);
        
        // Apply HPF
        hpFilters[ch].process (context);
        
        // Apply LPF
        lpFilters[ch].process (context);
    }
}

void TapMatrixAudioProcessor::applyDucking (juce::AudioBuffer<float>& wetBuffer, 
                                           const juce::AudioBuffer<float>& dryBuffer, 
                                           int numSamples)
{
    float duckingDb = parameters.getRawParameterValue ("ducking")->load();
    
    // Skip if ducking is disabled
    if (duckingDb < 0.1f)
        return;
    
    // Convert dB to linear reduction amount (0-12dB -> 0.0-1.0)
    float duckAmount = duckingDb / 12.0f;
    
    // Envelope follower parameters
    const float attackCoeff = 1.0f - std::exp (-1.0f / (0.001f * static_cast<float> (getSampleRate())));  // 1ms attack
    const float releaseCoeff = 1.0f - std::exp (-1.0f / (0.050f * static_cast<float> (getSampleRate()))); // 50ms release
    
    const int numChannels = wetBuffer.getNumChannels();
    
    for (int i = 0; i < numSamples; ++i)
    {
        // Calculate RMS squared of dry signal across all channels (avoid sqrt per sample)
        float dryEnergySq = 0.0f;
        for (int ch = 0; ch < juce::jmin (dryBuffer.getNumChannels(), MAX_CHANNELS); ++ch)
        {
            float sample = dryBuffer.getSample (ch, i);
            dryEnergySq += sample * sample;
        }
        dryEnergySq = dryEnergySq / juce::jmax (1, dryBuffer.getNumChannels());
        
        // Envelope follower (in squared domain)
        if (dryEnergySq > duckingEnvelopeSq)
            duckingEnvelopeSq += attackCoeff * (dryEnergySq - duckingEnvelopeSq);
        else
            duckingEnvelopeSq += releaseCoeff * (dryEnergySq - duckingEnvelopeSq);
        
        // Calculate ducking gain: wet *= 1 - (duckAmount * sqrt(envelope))
        // Only sqrt once for gain calculation (much faster than per-sample sqrt)
        float duckingGain = 1.0f - (duckAmount * std::sqrt (duckingEnvelopeSq));
        duckingGain = juce::jmax (0.0f, duckingGain);  // Prevent negative gain
        
        // Apply ducking to all wet channels
        for (int ch = 0; ch < juce::jmin (numChannels, MAX_CHANNELS); ++ch)
            wetBuffer.setSample (ch, i, wetBuffer.getSample (ch, i) * duckingGain);
    }
}

void TapMatrixAudioProcessor::applyDryWetMix (juce::AudioBuffer<float>& outputBuffer,
                                              const juce::AudioBuffer<float>& dryBuffer,
                                              const juce::AudioBuffer<float>& wetBuffer,
                                              int numSamples)
{
    float mix = parameters.getRawParameterValue ("mix")->load();
    
    const int numChannels = outputBuffer.getNumChannels();
    const int numDryChannels = dryBuffer.getNumChannels();
    
    // Handle channel mismatch (downmix/upmix)
    for (int ch = 0; ch < juce::jmin (numChannels, MAX_CHANNELS); ++ch)
    {
        auto* output = outputBuffer.getWritePointer (ch);
        
        // Get dry signal (handle channel count mismatch)
        const float* dryPtr = nullptr;
        if (ch < numDryChannels)
        {
            dryPtr = dryBuffer.getReadPointer (ch);
        }
        else if (numDryChannels > 0)
        {
            // Simple downmix: use channel 0 if we run out of dry channels
            dryPtr = dryBuffer.getReadPointer (0);
        }
        
        // Get wet signal
        const float* wetPtr = wetBuffer.getReadPointer (ch);
        
        // Mix: output = (1-mix) * dry + mix * wet
        if (dryPtr != nullptr)
        {
            for (int i = 0; i < numSamples; ++i)
                output[i] = (1.0f - mix) * dryPtr[i] + mix * wetPtr[i];
        }
        else
        {
            // No dry signal available, just use wet
            for (int i = 0; i < numSamples; ++i)
                output[i] = mix * wetPtr[i];
        }
    }
}

//==============================================================================
// Tape Mode - Cubic Interpolation
//==============================================================================

float TapMatrixAudioProcessor::cubicInterpolate (float y0, float y1, float y2, float y3, float frac)
{
    // 4-point, 3rd-order Hermite interpolation (x-form)
    // Provides smooth interpolation with minimal overshoot
    float c0 = y1;
    float c1 = 0.5f * (y2 - y0);
    float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
    float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
    
    return ((c3 * frac + c2) * frac + c1) * frac + c0;
}

//==============================================================================
// Factory Presets
//==============================================================================

void TapMatrixAudioProcessor::loadFactoryPreset (int presetIndex)
{
    // Helper lambda to set a tap parameter
    auto setTapParam = [this](int tapIndex, const char* paramName, float value)
    {
        if (auto* param = parameters.getParameter (getTapParamID (paramName, tapIndex)))
            param->setValueNotifyingHost (param->convertTo0to1 (value));
    };
    
    // Helper lambda to set a global parameter
    auto setGlobalParam = [this](const char* paramName, float value)
    {
        if (auto* param = parameters.getParameter (paramName))
            param->setValueNotifyingHost (param->convertTo0to1 (value));
    };
    
    switch (presetIndex)
    {
        case 0: // Init (Default)
        {
            // Reset all taps to clean slate
            for (int i = 0; i < NUM_TAPS; ++i)
            {
                setTapParam (i, "gain", 0.0f);
                setTapParam (i, "delayTime", 100.0f * (i + 1));
                setTapParam (i, "feedback", 0.0f);
                setTapParam (i, "crosstalk", 0.0f);
                setTapParam (i, "damping", 0.0f);
                setTapParam (i, "reverb", 0.0f);
                setTapParam (i, "panX", 0.0f);
                setTapParam (i, "panY", 0.0f);
                setTapParam (i, "panZ", 0.0f);
                setTapParam (i, "syncMode", 0.0f);
                setTapParam (i, "syncDelay", 0.25f * (i + 1));
            }
            setGlobalParam ("mix", 1.0f);
            setGlobalParam ("outputGain", 0.0f);
            setGlobalParam ("reverbType", 2.0f); // Medium
            setGlobalParam ("hpfFreq", 20.0f);
            setGlobalParam ("lpfFreq", 20000.0f);
            setGlobalParam ("ducking", 0.0f);
            setGlobalParam ("tapeMode", 1.0f);
            break;
        }
        
        case 1: // Vintage Slap
        {
            // Classic slap delay - short delays with some feedback and warm damping
            setTapParam (0, "gain", -3.0f);
            setTapParam (0, "delayTime", 80.0f);
            setTapParam (0, "feedback", 0.35f);
            setTapParam (0, "damping", 0.4f);
            setTapParam (0, "panX", -0.5f);
            
            setTapParam (1, "gain", -6.0f);
            setTapParam (1, "delayTime", 125.0f);
            setTapParam (1, "feedback", 0.25f);
            setTapParam (1, "damping", 0.5f);
            setTapParam (1, "panX", 0.5f);
            
            // Disable other taps
            for (int i = 2; i < NUM_TAPS; ++i)
                setTapParam (i, "gain", -96.0f);
            
            setGlobalParam ("mix", 0.35f);
            setGlobalParam ("lpfFreq", 5000.0f);
            setGlobalParam ("reverbType", 0.0f); // Dark
            break;
        }
        
        case 2: // Haas Widener
        {
            // Stereo widening using Haas effect (10-30ms delays)
            setTapParam (0, "gain", -3.0f);
            setTapParam (0, "delayTime", 15.0f);
            setTapParam (0, "panX", -1.0f);
            
            setTapParam (1, "gain", -3.0f);
            setTapParam (1, "delayTime", 15.0f);
            setTapParam (1, "panX", 1.0f);
            
            setTapParam (2, "gain", -9.0f);
            setTapParam (2, "delayTime", 25.0f);
            setTapParam (2, "panX", -0.7f);
            
            setTapParam (3, "gain", -9.0f);
            setTapParam (3, "delayTime", 25.0f);
            setTapParam (3, "panX", 0.7f);
            
            for (int i = 4; i < NUM_TAPS; ++i)
                setTapParam (i, "gain", -96.0f);
            
            setGlobalParam ("mix", 0.5f);
            break;
        }
        
        case 3: // Rhythmic Bounce
        {
            // Syncopated rhythm with alternating pans
            for (int i = 0; i < 6; ++i)
            {
                setTapParam (i, "gain", -6.0f - (i * 2.0f));
                setTapParam (i, "delayTime", 150.0f + (i * 100.0f));
                setTapParam (i, "feedback", 0.1f);
                setTapParam (i, "panX", (i % 2 == 0) ? -0.6f : 0.6f);
            }
            
            setTapParam (6, "gain", -96.0f);
            setTapParam (7, "gain", -96.0f);
            
            setGlobalParam ("mix", 0.45f);
            setGlobalParam ("tapeMode", 1.0f);
            break;
        }
        
        case 4: // Surround Wash
        {
            // Immersive surround delay with reverb
            for (int i = 0; i < NUM_TAPS; ++i)
            {
                float angle = (i / 8.0f) * juce::MathConstants<float>::twoPi;
                setTapParam (i, "gain", -9.0f);
                setTapParam (i, "delayTime", 200.0f + (i * 50.0f));
                setTapParam (i, "feedback", 0.4f);
                setTapParam (i, "reverb", 0.6f);
                setTapParam (i, "panX", std::cos (angle) * 0.9f);
                setTapParam (i, "panY", std::sin (angle) * 0.9f);
                setTapParam (i, "panZ", 0.3f);
            }
            
            setGlobalParam ("mix", 0.5f);
            setGlobalParam ("reverbType", 3.0f); // Long
            setGlobalParam ("lpfFreq", 8000.0f);
            break;
        }
        
        case 5: // Ping Pong Delay
        {
            // Classic ping-pong between left and right
            for (int i = 0; i < 6; ++i)
            {
                setTapParam (i, "gain", -6.0f - (i * 1.5f));
                setTapParam (i, "delayTime", 200.0f + (i * 200.0f));
                setTapParam (i, "feedback", 0.15f);
                setTapParam (i, "panX", (i % 2 == 0) ? -0.9f : 0.9f);
            }
            
            setTapParam (6, "gain", -96.0f);
            setTapParam (7, "gain", -96.0f);
            
            setGlobalParam ("mix", 0.4f);
            setGlobalParam ("tapeMode", 1.0f);
            break;
        }
        
        case 6: // Tight Doubler
        {
            // Very short delays for thickening
            setTapParam (0, "gain", -6.0f);
            setTapParam (0, "delayTime", 8.0f);
            setTapParam (0, "panX", -0.3f);
            
            setTapParam (1, "gain", -6.0f);
            setTapParam (1, "delayTime", 12.0f);
            setTapParam (1, "panX", 0.3f);
            
            setTapParam (2, "gain", -9.0f);
            setTapParam (2, "delayTime", 18.0f);
            setTapParam (2, "panX", -0.6f);
            
            setTapParam (3, "gain", -9.0f);
            setTapParam (3, "delayTime", 22.0f);
            setTapParam (3, "panX", 0.6f);
            
            for (int i = 4; i < NUM_TAPS; ++i)
                setTapParam (i, "gain", -96.0f);
            
            setGlobalParam ("mix", 0.3f);
            break;
        }
        
        case 7: // Spatial Echo
        {
            // All 8 taps active with crosstalk for complex echoes
            for (int i = 0; i < NUM_TAPS; ++i)
            {
                setTapParam (i, "gain", -9.0f);
                setTapParam (i, "delayTime", 100.0f + (i * 150.0f));
                setTapParam (i, "feedback", 0.3f);
                setTapParam (i, "crosstalk", 0.15f);
                setTapParam (i, "damping", 0.2f);
                setTapParam (i, "reverb", 0.3f);
                setTapParam (i, "panX", ((i % 4) - 1.5f) / 1.5f);
                setTapParam (i, "panY", ((i / 4) - 0.5f) * 0.8f);
            }
            
            setGlobalParam ("mix", 0.4f);
            setGlobalParam ("reverbType", 2.0f); // Medium
            break;
        }
    }
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TapMatrixAudioProcessor();
}
