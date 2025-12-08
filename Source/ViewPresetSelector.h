#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "SurroundStageView.h"

/**
 * Segmented control for selecting view presets
 * 
 * Features:
 * - 5 preset options: Angle, Left, Top, Right, Back
 * - Animated pill selector that slides between options
 * - Dims when view is in "Custom" state (user has dragged to rotate)
 */
class ViewPresetSelector : public juce::Component,
                           public juce::Timer
{
public:
    //==========================================================================
    // Styling constants
    static constexpr float cornerRadius = 5.0f;
    static constexpr float borderWidth = 1.0f;
    static constexpr float pillPadding = 2.0f;
    static constexpr float animationSpeed = 0.15f;  // 0-1 progress per frame
    
    // Colors
    static inline const juce::Colour borderColour {0xff4a4a4a};      /* #4a4a4a */
    static inline const juce::Colour pillColour {0xff5a5a5a};        /* #5a5a5a */
    static inline const juce::Colour pillDimmedColour {0xff3a3a3a};  /* #3a3a3a */
    static inline const juce::Colour textColour {0xffffffff};        /* #ffffff */
    static inline const juce::Colour textDimmedColour {0xff888888};  /* #888888 */
    static inline const juce::Colour backgroundColour {0xff2a2a2a};  /* #2a2a2a */
    
    //==========================================================================
    ViewPresetSelector();
    ~ViewPresetSelector() override;
    
    //==========================================================================
    // Set the callback for when a preset is selected
    std::function<void(SurroundStageView::ViewPreset)> onPresetSelected;
    
    // Update the current preset (call this when SurroundStageView's preset changes)
    void setCurrentPreset (SurroundStageView::ViewPreset preset);
    
    // Set whether the view is in custom (dragged) state
    void setCustomState (bool isCustom);
    
    //==========================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& event) override;
    
private:
    //==========================================================================
    // Preset labels
    static constexpr int numPresets = 5;
    const juce::String presetLabels[numPresets] = {"ANGLE", "LEFT", "TOP", "RIGHT", "BACK"};
    const SurroundStageView::ViewPreset presets[numPresets] = {
        SurroundStageView::ViewPreset::Angle,
        SurroundStageView::ViewPreset::Left,
        SurroundStageView::ViewPreset::Top,
        SurroundStageView::ViewPreset::Right,
        SurroundStageView::ViewPreset::Back
    };
    
    //==========================================================================
    // State
    int currentIndex = 0;           // Currently selected preset index
    float pillPosition = 0.0f;      // Animated position (0.0 to numPresets-1)
    float targetPosition = 0.0f;    // Target position for animation
    bool isCustom = false;          // Whether view has been manually rotated
    
    //==========================================================================
    // Animation
    void timerCallback() override;
    
    // Get the bounds for a specific segment
    juce::Rectangle<float> getSegmentBounds (int index) const;
    
    // Get the bounds for the pill at the current animated position
    juce::Rectangle<float> getPillBounds() const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ViewPresetSelector)
};
