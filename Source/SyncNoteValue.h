#pragma once

#include <juce_core/juce_core.h>

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
 * Get delay time in milliseconds for a note value at a given BPM.
 */
inline float getNoteValueMs (SyncNoteValue note, float bpm)
{
    if (bpm <= 0.0f)
        bpm = 120.0f;
    
    // Base: quarter note = 60000 / BPM (one beat in ms)
    float quarterNoteMs = 60000.0f / bpm;
    
    switch (note)
    {
        case SyncNoteValue::Note_1_1:   return quarterNoteMs * 4.0f;
        case SyncNoteValue::Note_1_2:   return quarterNoteMs * 2.0f;
        case SyncNoteValue::Note_1_4:   return quarterNoteMs;
        case SyncNoteValue::Note_1_8:   return quarterNoteMs * 0.5f;
        case SyncNoteValue::Note_1_16:  return quarterNoteMs * 0.25f;
        case SyncNoteValue::Note_1_32:  return quarterNoteMs * 0.125f;
        
        // Dotted = 1.5x
        case SyncNoteValue::Note_1_2D:  return quarterNoteMs * 3.0f;
        case SyncNoteValue::Note_1_4D:  return quarterNoteMs * 1.5f;
        case SyncNoteValue::Note_1_8D:  return quarterNoteMs * 0.75f;
        case SyncNoteValue::Note_1_16D: return quarterNoteMs * 0.375f;
        
        // Triplet = 2/3
        case SyncNoteValue::Note_1_2T:  return quarterNoteMs * 4.0f / 3.0f;
        case SyncNoteValue::Note_1_4T:  return quarterNoteMs * 2.0f / 3.0f;
        case SyncNoteValue::Note_1_8T:  return quarterNoteMs / 3.0f;
        case SyncNoteValue::Note_1_16T: return quarterNoteMs / 6.0f;
        case SyncNoteValue::Note_1_32T: return quarterNoteMs / 12.0f;
        
        case SyncNoteValue::NumValues:
        default: return quarterNoteMs;
    }
}

//==============================================================================
/**
 * Convert a millisecond value to the closest note value at the given BPM.
 */
inline SyncNoteValue msToNearestNoteValue (float ms, float bpm)
{
    if (bpm <= 0.0f)
        bpm = 120.0f;
    
    SyncNoteValue closest = SyncNoteValue::Note_1_4;
    float smallestDiff = std::abs (ms - getNoteValueMs (SyncNoteValue::Note_1_4, bpm));
    
    for (int i = 0; i < static_cast<int>(SyncNoteValue::NumValues); ++i)
    {
        auto note = static_cast<SyncNoteValue>(i);
        float diff = std::abs (ms - getNoteValueMs (note, bpm));
        if (diff < smallestDiff)
        {
            smallestDiff = diff;
            closest = note;
        }
    }
    
    return closest;
}
