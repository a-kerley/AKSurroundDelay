#include "PositionControlGroup.h"

//==============================================================================
// CONSTRUCTOR
//==============================================================================
PositionControlGroup::PositionControlGroup (int tapIndex_, juce::AudioProcessorValueTreeState& apvts)
    : tapIndex (tapIndex_)
{
    //==========================================================================
    // CONFIGURE LEFT/RIGHT HORIZONTAL FADER
    // Controls stereo pan position: -1 (full left) to +1 (full right)
    //==========================================================================
    leftRightFader.setValueDisplayMode (ValueDisplayMode::PanLeftRight);
    leftRightFader.setValueSuffix ("");       // No suffix for pan display
    leftRightFader.setDecimalPlaces (0);      // Integer pan values
    leftRightFader.setLabelFontSize (8.0f);   // Smaller font for compact layout
    leftRightFader.setLabelSpacing (1.0f);    // No spacing above label
    leftRightFader.setPaddingBottom (1.0f);   // No padding below label
    leftRightFader.setLabelHeight (8.0f);     // Tight label bounds (matches font size)
    addAndMakeVisible (leftRightFader);
    
    // Attach to parameter (e.g., "panX1", "panX2", etc.)
    juce::String lrParamID = "panX" + juce::String (tapIndex + 1);
    if (apvts.getParameter (lrParamID) != nullptr)
        leftRightFader.attachToParameter (apvts, lrParamID);
    
    //==========================================================================
    // CONFIGURE FRONT/BACK VERTICAL FADER  
    // Controls depth position: -1 (back) to +1 (front), C in middle
    //==========================================================================
    frontBackFader.setValueDisplayMode (ValueDisplayMode::FrontBack);
    frontBackFader.setValueSuffix ("");       // No suffix - mode handles formatting
    frontBackFader.setDecimalPlaces (0);
    frontBackFader.setLabelFontSize (8.0f);   // Smaller font for compact layout
    frontBackFader.setLabelSpacing (1.0f);    // No spacing above label
    frontBackFader.setPaddingBottom (1.0f);   // No padding below label
    frontBackFader.setLabelHeight (8.0f);     // Tight label bounds (matches font size)
    addAndMakeVisible (frontBackFader);
    
    // Attach to parameter (e.g., "panY1", "panY2", etc.)
    juce::String fbParamID = "panY" + juce::String (tapIndex + 1);
    if (apvts.getParameter (fbParamID) != nullptr)
        frontBackFader.attachToParameter (apvts, fbParamID);
    
    //==========================================================================
    // CONFIGURE HEIGHT VERTICAL FADER
    // Controls vertical position: 0% (floor) to 100% (ceiling)
    // May be disabled if channel format doesn't support height
    //==========================================================================
    heightFader.setValueDisplayMode (ValueDisplayMode::Percent);
    heightFader.setValueSuffix ("");          // No suffix - mode handles formatting
    heightFader.setDecimalPlaces (0);
    heightFader.setLabelFontSize (8.0f);      // Smaller font for compact layout
    heightFader.setLabelSpacing (1.0f);       // No spacing above label
    heightFader.setPaddingBottom (1.0f);      // No padding below label
    heightFader.setLabelHeight (8.0f);        // Tight label bounds (matches font size)
    addAndMakeVisible (heightFader);
    
    // Attach to parameter (e.g., "panZ1", "panZ2", etc.)
    juce::String heightParamID = "panZ" + juce::String (tapIndex + 1);
    if (apvts.getParameter (heightParamID) != nullptr)
        heightFader.attachToParameter (apvts, heightParamID);
    
    // Start with height disabled by default (can be enabled if format supports it)
    setHeightEnabled (false);
}

//==============================================================================
// PAINT
//==============================================================================
void PositionControlGroup::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Draw the "POSITION" group label at the bottom
    auto groupLabelBounds = bounds.removeFromBottom ((int)scaled (baseGroupLabelHeight));
    
    g.setFont (juce::FontOptions (scaled (baseGroupLabelFontSize)));
    g.setColour (ColorPalette::groupLabelColour);
    g.drawText ("POSITION", groupLabelBounds, juce::Justification::centredTop);
}

//==============================================================================
// RESIZED
// 
// Layout strategy:
// 1. Calculate positions top-down within the 170px control area
// 2. L/R fader at top, centered horizontally
// 3. F/B and Height faders side-by-side below, centered as a pair
// 4. Group label below the control area
//==============================================================================
void PositionControlGroup::resized()
{
    auto bounds = getLocalBounds();
    
    //==========================================================================
    // REMOVE GROUP LABEL AREA FROM BOTTOM
    //==========================================================================
    bounds.removeFromBottom ((int)scaled (baseGroupLabelHeight));
    bounds.removeFromBottom ((int)scaled (baseGroupLabelSpacing));
    
    // Now 'bounds' is the 170px control area
    auto controlArea = bounds;
    
    //==========================================================================
    // CALCULATE L/R FADER POSITION (top, centered)
    // The SliderModule adds its own padding around the track
    //==========================================================================
    int lrPreferredWidth = leftRightFader.getPreferredWidth();
    int lrPreferredHeight = leftRightFader.getPreferredHeight();
    
    // Position at top, centered horizontally
    int lrX = (controlArea.getWidth() - lrPreferredWidth) / 2;
    int lrY = controlArea.getY();
    leftRightFader.setBounds (lrX, lrY, lrPreferredWidth, lrPreferredHeight);
    
    //==========================================================================
    // CALCULATE VERTICAL FADERS POSITION (below L/R, side-by-side)
    // 
    // The L/R SliderModule has internal padding (top padding + label + bottom padding).
    // To position vertical faders closer, we calculate where the L/R LABEL ends,
    // not where the full component ends.
    // 
    // L/R component layout (vertical):
    //   - componentPaddingTop (8px)
    //   - track (28px for horizontal fader's short dimension) 
    //   - labelSpacing (4px)
    //   - label (14px)
    //   - componentPaddingBottom (4px)
    //
    // We want vertical faders to start at: L/R label bottom + baseVerticalGap
    //==========================================================================
    int fbPreferredWidth = frontBackFader.getPreferredWidth();
    int fbPreferredHeight = frontBackFader.getPreferredHeight();
    int heightPreferredWidth = heightFader.getPreferredWidth();
    int heightPreferredHeight = heightFader.getPreferredHeight();
    
    // Calculate where the L/R fader's label ends
    // The vertical faders also have top padding, so we subtract that to align the tracks better
    float verticalFaderTopPadding = frontBackFader.componentPaddingTop();
    
    // Position vertical faders: start after L/R fader + gap - vertical fader's top padding
    // This makes the vertical fader TRACKS appear closer to the L/R label
    int verticalFadersTop = lrY + lrPreferredHeight + (int)scaled (baseVerticalGap) - (int)verticalFaderTopPadding;
    
    // Total width of both vertical faders + gap between them
    int verticalPairWidth = fbPreferredWidth + (int)scaled (baseHorizontalGap) + heightPreferredWidth;
    
    // Center the pair horizontally
    int verticalPairX = (controlArea.getWidth() - verticalPairWidth) / 2;
    
    // Front/Back fader on the left
    int fbX = verticalPairX;
    int fbY = verticalFadersTop;
    frontBackFader.setBounds (fbX, fbY, fbPreferredWidth, fbPreferredHeight);
    
    // Height fader on the right
    int heightX = fbX + fbPreferredWidth + (int)scaled (baseHorizontalGap);
    int heightY = verticalFadersTop;
    heightFader.setBounds (heightX, heightY, heightPreferredWidth, heightPreferredHeight);
}

//==============================================================================
// SCALING
//==============================================================================
void PositionControlGroup::setScaleFactor (float scale)
{
    if (juce::approximatelyEqual (currentScaleFactor, scale))
        return;
    
    currentScaleFactor = scale;
    
    // Update all child sliders
    leftRightFader.setScaleFactor (scale);
    frontBackFader.setScaleFactor (scale);
    heightFader.setScaleFactor (scale);
    
    // Re-layout
    resized();
    repaint();
}

//==============================================================================
// DIMENSIONS
//==============================================================================
int PositionControlGroup::getPreferredWidth() const
{
    // Width is determined by the wider of:
    // 1. The L/R horizontal fader
    // 2. The pair of vertical faders + gap
    
    int lrWidth = leftRightFader.getPreferredWidth();
    int verticalPairWidth = frontBackFader.getPreferredWidth() 
                          + (int)scaled (baseHorizontalGap) 
                          + heightFader.getPreferredWidth();
    
    return juce::jmax (lrWidth, verticalPairWidth);
}

int PositionControlGroup::getPreferredHeight() const
{
    // Total height = control area (170px) + gap + group label
    return (int)(scaled (baseTotalControlHeight) 
               + scaled (baseGroupLabelSpacing) 
               + scaled (baseGroupLabelHeight));
}

//==============================================================================
// HEIGHT ENABLE/DISABLE
//==============================================================================
void PositionControlGroup::setHeightEnabled (bool enabled)
{
    if (heightEnabled == enabled)
        return;
    
    heightEnabled = enabled;
    heightFader.setSliderEnabled (enabled);
}

//==============================================================================
// ACCENT COLOR
//==============================================================================
void PositionControlGroup::setAccentColour (juce::Colour colour)
{
    if (accentColour == colour)
        return;
    
    accentColour = colour;
    
    // Apply accent color to all child faders (for track tinting)
    leftRightFader.setAccentColour (colour);
    frontBackFader.setAccentColour (colour);
    heightFader.setAccentColour (colour);
    
    repaint();
}

void PositionControlGroup::setValueTextColour (juce::Colour colour)
{
    // Set the value text color for all faders (for readable value labels)
    leftRightFader.setValueTextColour (colour);
    frontBackFader.setValueTextColour (colour);
    heightFader.setValueTextColour (colour);
    
    repaint();
}

//==============================================================================
// LOOK AND FEEL
//==============================================================================
void PositionControlGroup::setSliderLookAndFeel (juce::LookAndFeel* lf)
{
    leftRightFader.setLookAndFeel (lf);
    frontBackFader.setLookAndFeel (lf);
    heightFader.setLookAndFeel (lf);
}
