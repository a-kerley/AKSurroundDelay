#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>

//==============================================================================
/**
 * Single delay tap with feedback, crosstalk, damping, and reverb
 */
struct DelayTap
{
    // Per-tap circular buffer
    juce::AudioBuffer<float> buffer;
    int writePosition = 0;
    int bufferLength = 0;
    int bufferMask = 0;  // For fast power-of-2 wrapping
    
    // Per-tap state
    float lastOutputSample = 0.0f;
    float dampingCoeff = 1.0f;  // 1.0 = no damping, 0.0 = full damping
    
    // Tape mode - smooth delay time changes
    float currentDelaySamples = 0.0f;  // Current smoothed delay time
    float targetDelaySamples = 0.0f;   // Target delay time
    
    // Tempo sync state (stored separately from TIME mode)
    bool useSyncMode = false;           // true = SYNC (beats), false = TIME (ms)
    float syncDelayBeats = 0.0f;        // Delay in quarter notes (0-10)
    
    // Per-tap mono reverb instance
    juce::dsp::Reverb reverb;
    
    // Level metering (pre-pan)
    std::atomic<float> currentLevel { 0.0f };  // RMS level for UI display
    
    void prepareToPlay (double sampleRate, int maxDelayMs)
    {
        // Use power-of-2 for fast wrapping with bitmask
        int desiredLength = static_cast<int> (sampleRate * maxDelayMs / 1000.0);
        bufferLength = juce::nextPowerOfTwo (desiredLength);
        bufferMask = bufferLength - 1;
        buffer.setSize (1, bufferLength);  // Mono buffer per tap
        buffer.clear();
        writePosition = 0;
        lastOutputSample = 0.0f;
        currentDelaySamples = 0.0f;
        targetDelaySamples = 0.0f;
    }
    
    void reset()
    {
        buffer.clear();
        writePosition = 0;
        lastOutputSample = 0.0f;
        currentDelaySamples = 0.0f;
        targetDelaySamples = 0.0f;
    }
};

//==============================================================================
/**
 * Global Reverb Type Selection
 */
enum class ReverbType
{
    Dark = 0,
    Short,
    Medium,
    Long,
    XXXL
};

//==============================================================================
/**
 * TapMatrix Audio Processor
 * 
 * 8-tap spatial delay plugin with:
 * - Independent delay taps with feedback and crosstalk
 * - Per-tap 3D panning (XYZ)
 * - Per-tap reverb
 * - Global filtering and ducking
 * - Tape mode for smooth delay modulation
 */
class TapMatrixAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    TapMatrixAudioProcessor();
    ~TapMatrixAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    // Helper: Convert quarter notes to milliseconds at given BPM
    static float beatsToMs (float quarterNotes, double bpm)
    {
        if (bpm <= 0.0) return 0.0f;
        return static_cast<float> ((quarterNotes * 60000.0) / bpm);
    }

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
    
    // Get tap level for metering (0.0 to 1.0)
    float getTapLevel (int tapIndex) const
    {
        if (tapIndex >= 0 && tapIndex < NUM_TAPS)
            return taps[tapIndex].currentLevel.load();
        return 0.0f;
    }
    
    //==============================================================================
    // Factory presets
    void loadFactoryPreset (int presetIndex);
    static constexpr int NUM_FACTORY_PRESETS = 8;

private:
    //==============================================================================
    static constexpr int NUM_TAPS = 8;
    static constexpr int MAX_DELAY_MS = 2500;
    
    // Parameter tree state for automation and preset management
    juce::AudioProcessorValueTreeState parameters;
    
    // 8 independent delay taps
    std::array<DelayTap, NUM_TAPS> taps;
    
    // Mono sum buffer (input is always summed to mono)
    juce::AudioBuffer<float> monoInputBuffer;
    
    // Temporary buffer for tap outputs before panning
    juce::AudioBuffer<float> tapOutputBuffer;
    
    // Pre-allocated crosstalk buffer (avoid real-time allocation)
    juce::AudioBuffer<float> crosstalkBuffer;
    
    // Crosstalk matrix (8x8, diagonal is zero)
    std::array<std::array<float, NUM_TAPS>, NUM_TAPS> crosstalkMatrix;
    
    // Global processing chain
    // HPF/LPF filters (12dB/oct = 2-pole = StateVariableFilter)
    static constexpr int MAX_CHANNELS = 8;  // Support up to 7.1
    std::array<juce::dsp::StateVariableTPTFilter<float>, MAX_CHANNELS> hpFilters;
    std::array<juce::dsp::StateVariableTPTFilter<float>, MAX_CHANNELS> lpFilters;
    
    // Dry signal buffer for mixing
    juce::AudioBuffer<float> dryBuffer;
    
    // Ducking envelope follower state (squared domain for performance)
    float duckingEnvelopeSq = 0.0f;
    
    // Thread-safe reverb parameter updates
    std::atomic<bool> reverbParamsNeedUpdate { false };
    
    // Helper functions
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void processTaps (const float* monoInput, int numSamples, double bpm);
    void applyCrosstalk (int numSamples);
    void applyPanning (juce::AudioBuffer<float>& outputBuffer, int numSamples);
    
    // 3D Panning helpers
    void panTapToStereo (int tapIndex, float* leftOut, float* rightOut, int numSamples);
    void panTapTo51 (int tapIndex, juce::AudioBuffer<float>& outputBuffer, int numSamples);
    void panTapTo71 (int tapIndex, juce::AudioBuffer<float>& outputBuffer, int numSamples);
    
    // Reverb configuration
    void updateReverbParameters();
    juce::dsp::Reverb::Parameters getReverbPreset (ReverbType type) const;
    
    // Global processing
    void applyGlobalFilters (juce::AudioBuffer<float>& buffer, int numSamples);
    void applyDucking (juce::AudioBuffer<float>& wetBuffer, const juce::AudioBuffer<float>& dryBuffer, int numSamples);
    void applyDryWetMix (juce::AudioBuffer<float>& outputBuffer, const juce::AudioBuffer<float>& dryBuffer, 
                         const juce::AudioBuffer<float>& wetBuffer, int numSamples);
    
    // Tape mode - cubic interpolation helper
    static float cubicInterpolate (float y0, float y1, float y2, float y3, float frac);
    
    // Current reverb type
    ReverbType currentReverbType = ReverbType::Medium;
    
    // Current preset index
    int currentPresetIndex = 0;
    
    // Parameter ID generation helpers
    static juce::String getTapParamID (const char* paramName, int tapIndex);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapMatrixAudioProcessor)
};
