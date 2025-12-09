#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "SliderModule.h"
#include "PositionControlGroup.h"
#include "ColorPalette.h"

//==============================================================================
/**
 * PowerToggle - Small power button for tap enable/bypass
 * 
 * Displays a simple power icon (⏻) that toggles on/off state.
 * Color matches the parent TapPanel's accent color.
 */
class PowerToggle : public juce::Button
{
public:
    PowerToggle() : juce::Button ("PowerToggle") {}
    
    void setAccentColour (juce::Colour colour) 
    { 
        accentColour = colour; 
        repaint(); 
    }
    
    void paintButton (juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (2.0f);
        float size = juce::jmin (bounds.getWidth(), bounds.getHeight());
        auto iconBounds = bounds.withSizeKeepingCentre (size, size);
        
        // Draw power icon
        bool isOn = getToggleState();
        auto colour = isOn ? accentColour : accentColour.withAlpha (0.3f);
        
        if (shouldDrawButtonAsHighlighted)
            colour = colour.brighter (0.2f);
        if (shouldDrawButtonAsDown)
            colour = colour.darker (0.2f);
        
        g.setColour (colour);
        
        // Draw circle arc (power symbol)
        float centerX = iconBounds.getCentreX();
        float centerY = iconBounds.getCentreY();
        float radius = size * 0.35f;
        float lineThickness = size * 0.12f;
        
        juce::Path arc;
        arc.addCentredArc (centerX, centerY, radius, radius, 0.0f,
                           juce::MathConstants<float>::pi * 0.25f,
                           juce::MathConstants<float>::pi * 1.75f,
                           true);
        g.strokePath (arc, juce::PathStrokeType (lineThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        
        // Draw vertical line at top
        float lineHeight = size * 0.35f;
        g.drawLine (centerX, centerY - radius - lineThickness * 0.8f,
                    centerX, centerY - radius + lineHeight,
                    lineThickness);
    }
    
private:
    juce::Colour accentColour { 0xffffffff };
};

//==============================================================================
/**
 * TapPanel - A single tap's control panel
 * 
 * Contains all controls for one delay tap:
 * - Header with "TAP N" label and power toggle
 * - Level fader
 * - Time fader
 * - (Future: Position controls, Filter, Diffuse, Drive, Reverb send)
 * 
 * Each tap has its own accent color from the ColorPalette (0-7 → colors 0-7).
 * All 8 panels exist simultaneously; visibility is controlled by TapTabBar selection.
 */
class TapPanel : public juce::Component
{
public:
    /** Create a tap panel for the specified tap index (0-7) */
    TapPanel (int tapIndex, juce::AudioProcessorValueTreeState& apvts);
    ~TapPanel() override;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    /** Get this tap's index (0-7) */
    int getTapIndex() const { return tapIndex; }
    
    /** Get this tap's accent color */
    juce::Colour getAccentColour() const { return accentColour; }
    
    /** Set the UI scale factor */
    void setScaleFactor (float scale);
    float getScaleFactor() const { return currentScaleFactor; }
    
    /** Get preferred panel height based on fader size + header + padding */
    int getPreferredHeight() const;
    
    /** Get/set enabled state (power toggle) */
    bool isTapEnabled() const { return powerToggle.getToggleState(); }
    void setTapEnabled (bool enabled) { powerToggle.setToggleState (enabled, juce::dontSendNotification); }
    
    /** Set the custom LookAndFeel for child sliders */
    void setSliderLookAndFeel (juce::LookAndFeel* lf);
    
private:
    int tapIndex;                    // 0-7
    juce::Colour accentColour;       // From ColorPalette
    juce::Colour textColour;         // Matching text color from ColorPalette
    float currentScaleFactor = 1.0f;
    
    // Header layout constants (base, unscaled)
    static constexpr float baseHeaderHeight = 28.0f;
    static constexpr float baseBorderWidth = 1.5f;
    static constexpr float baseCornerRadius = 6.0f;
    static constexpr float basePadding = 10.0f;
    static constexpr float basePowerButtonSize = 20.0f;
    static constexpr float baseHeaderFontSize = 12.0f;
    
    // Header components
    PowerToggle powerToggle;
    
    // Faders - using standard 38x170 full-length faders
    SliderModule levelFader {"LEVEL", FaderStyle::Fader_38x170};
    SliderModule timeFader {"TIME", FaderStyle::Fader_38x170};
    
    // Position control group (L/R, F/B, Height)
    std::unique_ptr<PositionControlGroup> positionGroup;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapPanel)
};

//==============================================================================
/**
 * TapTabBar - Tab strip for selecting which TapPanel is visible
 * 
 * Displays 8 tabs (TAP 1 - TAP 8), each color-coded to match its TapPanel.
 * Clicking a tab selects it and notifies the parent to show the corresponding panel.
 */
class TapTabBar : public juce::Component
{
public:
    TapTabBar();
    ~TapTabBar() override = default;
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& event) override;
    
    /** Get the currently selected tap index (0-7) */
    int getSelectedTab() const { return selectedTab; }
    
    /** Set the currently selected tab (0-7) */
    void setSelectedTab (int index);
    
    /** Callback when a tab is selected */
    std::function<void (int)> onTabSelected;
    
    /** Set the UI scale factor */
    void setScaleFactor (float scale);
    
private:
    static constexpr int NUM_TABS = 8;
    int selectedTab = 0;
    float currentScaleFactor = 1.0f;
    
    // Layout constants (base, unscaled)
    static constexpr float baseTabHeight = 22.0f;
    static constexpr float baseTabSpacing = 4.0f;
    static constexpr float baseCornerRadius = 4.0f;
    static constexpr float baseFontSize = 10.0f;
    static constexpr float basePowerIconSize = 9.0f;
    static constexpr float baseHorizontalPadding = 12.0f;  // Left/right padding for tab bar
    
    // Cached tab bounds for hit testing
    std::array<juce::Rectangle<int>, NUM_TABS> tabBounds;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapTabBar)
};
