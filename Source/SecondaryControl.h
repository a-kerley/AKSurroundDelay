#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "ColorPalette.h"

//==============================================================================
/**
 * SECONDARY CONTROL TYPE ENUMERATION
 * 
 * Defines the types of secondary controls that can float on a slider track.
 * Each type has its own UI and interaction behavior.
 */
enum class SecondaryControlType
{
    None,           // No secondary control (default)
    SyncToggle,     // Tempo sync on/off toggle (icon + note value when on)
    FilterSlope     // Filter slope selector (dropdown: 6dB, 12dB, 24dB/oct)
    // Future: ModDepth, ModRate, etc.
};

//==============================================================================
/**
 * NOTE VALUE ENUMERATION
 * 
 * Musical note divisions for tempo sync.
 * Ordered from longest (1/1) to shortest (1/32), with triplet and dotted variants.
 */
enum class SyncNoteValue
{
    // Standard divisions
    Note_1_1,       // Whole note
    Note_1_2,       // Half note
    Note_1_4,       // Quarter note
    Note_1_8,       // Eighth note
    Note_1_16,      // Sixteenth note
    Note_1_32,      // Thirty-second note
    
    // Dotted variants (1.5x duration)
    Note_1_2D,      // Dotted half
    Note_1_4D,      // Dotted quarter
    Note_1_8D,      // Dotted eighth
    Note_1_16D,     // Dotted sixteenth
    
    // Triplet variants (2/3 duration)
    Note_1_2T,      // Half triplet
    Note_1_4T,      // Quarter triplet
    Note_1_8T,      // Eighth triplet
    Note_1_16T,     // Sixteenth triplet
    Note_1_32T,     // Thirty-second triplet
    
    NumValues
};

//==============================================================================
/**
 * Get display string for a note value (e.g., "1/4", "1/8T", "1/4D")
 */
inline juce::String getNoteValueString (SyncNoteValue note)
{
    switch (note)
    {
        case SyncNoteValue::Note_1_1:   return "1/1";
        case SyncNoteValue::Note_1_2:   return "1/2";
        case SyncNoteValue::Note_1_4:   return "1/4";
        case SyncNoteValue::Note_1_8:   return "1/8";
        case SyncNoteValue::Note_1_16:  return "1/16";
        case SyncNoteValue::Note_1_32:  return "1/32";
        case SyncNoteValue::Note_1_2D:  return "1/2D";
        case SyncNoteValue::Note_1_4D:  return "1/4D";
        case SyncNoteValue::Note_1_8D:  return "1/8D";
        case SyncNoteValue::Note_1_16D: return "1/16D";
        case SyncNoteValue::Note_1_2T:  return "1/2T";
        case SyncNoteValue::Note_1_4T:  return "1/4T";
        case SyncNoteValue::Note_1_8T:  return "1/8T";
        case SyncNoteValue::Note_1_16T: return "1/16T";
        case SyncNoteValue::Note_1_32T: return "1/32T";
        case SyncNoteValue::NumValues:
        default: return "1/4";
    }
}

//==============================================================================
/**
 * Get the multiplier for a note value relative to a quarter note.
 * Quarter note = 1.0, half = 2.0, eighth = 0.5, etc.
 * At 120 BPM, quarter note = 500ms.
 */
inline float getNoteValueMultiplier (SyncNoteValue note)
{
    switch (note)
    {
        case SyncNoteValue::Note_1_1:   return 4.0f;
        case SyncNoteValue::Note_1_2:   return 2.0f;
        case SyncNoteValue::Note_1_4:   return 1.0f;
        case SyncNoteValue::Note_1_8:   return 0.5f;
        case SyncNoteValue::Note_1_16:  return 0.25f;
        case SyncNoteValue::Note_1_32:  return 0.125f;
        case SyncNoteValue::Note_1_2D:  return 3.0f;      // 2 × 1.5
        case SyncNoteValue::Note_1_4D:  return 1.5f;      // 1 × 1.5
        case SyncNoteValue::Note_1_8D:  return 0.75f;     // 0.5 × 1.5
        case SyncNoteValue::Note_1_16D: return 0.375f;    // 0.25 × 1.5
        case SyncNoteValue::Note_1_2T:  return 1.333333f; // 2 × 2/3
        case SyncNoteValue::Note_1_4T:  return 0.666667f; // 1 × 2/3
        case SyncNoteValue::Note_1_8T:  return 0.333333f; // 0.5 × 2/3
        case SyncNoteValue::Note_1_16T: return 0.166667f; // 0.25 × 2/3
        case SyncNoteValue::Note_1_32T: return 0.083333f; // 0.125 × 2/3
        case SyncNoteValue::NumValues:
        default: return 1.0f;
    }
}

//==============================================================================
/**
 * Calculate delay time in milliseconds for a note value at a given BPM.
 * @param note The note value (1/4, 1/8T, etc.)
 * @param bpm The tempo in beats per minute
 * @return Delay time in milliseconds
 */
inline float getDelayTimeForNote (SyncNoteValue note, float bpm)
{
    if (bpm <= 0.0f) bpm = 120.0f;  // Fallback
    float quarterNoteMs = 60000.0f / bpm;  // Duration of one quarter note in ms
    return quarterNoteMs * getNoteValueMultiplier (note);
}

//==============================================================================
/**
 * Find the closest note value for a given delay time at a given BPM.
 * @param delayMs Delay time in milliseconds
 * @param bpm The tempo in beats per minute
 * @return The closest matching SyncNoteValue
 */
inline SyncNoteValue findClosestNoteValue (float delayMs, float bpm)
{
    if (bpm <= 0.0f) bpm = 120.0f;
    
    SyncNoteValue closest = SyncNoteValue::Note_1_4;
    float closestDiff = std::abs (delayMs - getDelayTimeForNote (SyncNoteValue::Note_1_4, bpm));
    
    for (int i = 0; i < static_cast<int>(SyncNoteValue::NumValues); ++i)
    {
        auto noteVal = static_cast<SyncNoteValue>(i);
        float noteMs = getDelayTimeForNote (noteVal, bpm);
        float diff = std::abs (delayMs - noteMs);
        if (diff < closestDiff)
        {
            closestDiff = diff;
            closest = noteVal;
        }
    }
    return closest;
}

//==============================================================================
/**
 * SecondarySliderControl - Base class for floating controls on slider tracks.
 * 
 * These controls float on the slider track, positioned relative to the thumb.
 * They automatically switch between "below thumb" and "above thumb" positions
 * based on available space, and change their color accordingly:
 * - Below thumb (on fill area): uses text color (darker, readable on bright fill)
 * - Above thumb (on empty track): uses accent color (brighter, visible on dark track)
 * 
 * Layout (vertical slider):
 * ┌─────────────────┐
 * │ (empty track)   │  ← Control appears here when near bottom (uses accent color)
 * │                 │
 * │ ┌─CONTROL─┐     │  ← Control (above thumb)
 * │ └─────────┘     │
 * │    [THUMB]      │  ← Current thumb position
 * │ ┌─CONTROL─┐     │  ← Control (below thumb, preferred)
 * │ └─────────┘     │
 * │ (fill area)     │  ← Control appears here when near top (uses text color)
 * └─────────────────┘
 * 
 * Subclasses implement specific control types (toggle, dropdown, etc.)
 */
class SecondarySliderControl : public juce::Component
{
public:
    //==========================================================================
    // CONSTANTS (base, unscaled values)
    //==========================================================================
    static constexpr float baseControlHeight = 16.0f;     // Height of control area
    static constexpr float baseThumbGap = 4.0f;           // Gap between thumb and control
    static constexpr float baseIconSize = 12.0f;          // Icon size
    static constexpr float baseFontSize = 8.0f;           // Font size for text
    static constexpr float baseCornerRadius = 3.0f;       // Corner radius for backgrounds
    
    //==========================================================================
    // CONSTRUCTOR & DESTRUCTOR
    //==========================================================================
    SecondarySliderControl();
    virtual ~SecondarySliderControl() override = default;
    
    //==========================================================================
    // CONFIGURATION
    //==========================================================================
    /** Set the scale factor for this control */
    void setScaleFactor (float scale) { scaleFactor = scale; resized(); repaint(); }
    float getScaleFactor() const { return scaleFactor; }
    
    /** Set the accent color (used when control is above thumb) */
    void setAccentColour (juce::Colour colour) { accentColour = colour; repaint(); }
    juce::Colour getAccentColour() const { return accentColour; }
    
    /** Set the text color (used when control is below thumb, on fill) */
    void setTextColour (juce::Colour colour) { textColour = colour; repaint(); }
    juce::Colour getTextColour() const { return textColour; }
    
    /** Set whether the control is currently positioned above the thumb */
    void setAboveThumb (bool above) { isAboveThumb = above; repaint(); }
    bool getIsAboveThumb() const { return isAboveThumb; }
    
    /** Get the current drawing color based on position */
    juce::Colour getCurrentColour() const 
    { 
        return isAboveThumb ? accentColour : textColour; 
    }
    
    /** Get the scaled control height */
    float getControlHeight() const { return baseControlHeight * scaleFactor; }
    
    /** Get the scaled gap from thumb */
    float getThumbGap() const { return baseThumbGap * scaleFactor; }
    
    //==========================================================================
    // ABSTRACT METHODS - Implement in subclasses
    //==========================================================================
    /** Get the preferred width for this control (scaled) */
    virtual float getPreferredWidth() const = 0;
    
    /** Called when the control is clicked */
    virtual void controlClicked() = 0;
    
    //==========================================================================
    // COMPONENT OVERRIDES
    //==========================================================================
    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseEnter (const juce::MouseEvent& event) override;
    void mouseExit (const juce::MouseEvent& event) override;
    
protected:
    float scaleFactor = 1.0f;
    juce::Colour accentColour { 0xffffffff };  /* #ffffff - default white */
    juce::Colour textColour { 0xff1a1a1a };    /* #1a1a1a - default dark */
    bool isAboveThumb = false;                  // Position state
    bool isHovered = false;                     // Mouse hover state
};

//==============================================================================
/**
 * SyncToggleControl - Tempo sync toggle for delay time sliders.
 * 
 * Displays "SYNC" label with a clock icon:
 * - When OFF: Outline clock icon (dimmed)
 * - When ON: Filled clock icon (bright)
 * 
 * The "SYNC" text remains constant color (accent color).
 * When sync is enabled, the parent slider's value display changes to show
 * note durations (1/4, 1/8T, etc.) instead of milliseconds.
 * 
 * Click to toggle sync on/off.
 */
class SyncToggleControl : public SecondarySliderControl
{
public:
    //==========================================================================
    // CONSTANTS
    //==========================================================================
    static constexpr float baseSyncTextHeight = 10.0f;  // Height for "SYNC" text row
    static constexpr float baseGap = 2.0f;              // Gap between text and icon
    static constexpr float baseControlWidth = 28.0f;    // Total control width
    
    //==========================================================================
    // CONSTRUCTOR
    //==========================================================================
    SyncToggleControl();
    
    //==========================================================================
    // CONFIGURATION
    //==========================================================================
    /** Set sync enabled state */
    void setSyncEnabled (bool enabled) { syncEnabled = enabled; repaint(); }
    bool isSyncEnabled() const { return syncEnabled; }
    
    /** Set current note value (stored for reference, displayed on fader) */
    void setNoteValue (SyncNoteValue note) { currentNoteValue = note; }
    SyncNoteValue getNoteValue() const { return currentNoteValue; }
    
    /** Callback when sync is toggled */
    std::function<void(bool)> onSyncToggled;
    
    //==========================================================================
    // OVERRIDES
    //==========================================================================
    float getPreferredWidth() const override;
    float getControlHeight() const;  // Custom height for vertical layout
    void controlClicked() override;
    void paint (juce::Graphics& g) override;
    
private:
    bool syncEnabled = false;
    SyncNoteValue currentNoteValue = SyncNoteValue::Note_1_4;
    
    /** Draw the clock icon - outline when off, filled when on */
    void drawClockIcon (juce::Graphics& g, juce::Rectangle<float> bounds, 
                        juce::Colour colour, bool filled);
};

