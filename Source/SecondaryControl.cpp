#include "SecondaryControl.h"

//==============================================================================
// SECONDARY SLIDER CONTROL - Base class implementation
//==============================================================================

SecondarySliderControl::SecondarySliderControl()
{
    setInterceptsMouseClicks (true, false);
    setPaintingIsUnclipped (true);  // Allow painting outside parent bounds
}

void SecondarySliderControl::paint (juce::Graphics& g)
{
    // Base class just fills with current color for debugging
    // Subclasses override this completely
    auto bounds = getLocalBounds().toFloat();
    auto colour = getCurrentColour();
    
    if (isHovered)
        colour = colour.brighter (0.2f);
    
    g.setColour (colour.withAlpha (0.1f));
    g.fillRoundedRectangle (bounds, baseCornerRadius * scaleFactor);
}

void SecondarySliderControl::mouseDown (const juce::MouseEvent& /*event*/)
{
    controlClicked();
}

void SecondarySliderControl::mouseEnter (const juce::MouseEvent& /*event*/)
{
    isHovered = true;
    repaint();
}

void SecondarySliderControl::mouseExit (const juce::MouseEvent& /*event*/)
{
    isHovered = false;
    repaint();
}

//==============================================================================
// SYNC TOGGLE CONTROL - Implementation
//==============================================================================

SyncToggleControl::SyncToggleControl()
{
}

float SyncToggleControl::getPreferredWidth() const
{
    // Vertical layout: SYNC text on top, icon below
    return baseControlWidth * scaleFactor;
}

float SyncToggleControl::getControlHeight() const
{
    // Stack: SYNC text + gap + icon
    float textHeight = baseSyncTextHeight * scaleFactor;
    float iconHeight = baseIconSize * scaleFactor;
    float gap = baseGap * scaleFactor;
    
    return textHeight + gap + iconHeight;
}

void SyncToggleControl::controlClicked()
{
    syncEnabled = !syncEnabled;
    repaint();
    
    if (onSyncToggled)
        onSyncToggled (syncEnabled);
}

void SyncToggleControl::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // The accent color is always used for "SYNC" text (doesn't change on toggle)
    // Icon uses current color which changes based on above/below thumb position
    auto iconColour = getCurrentColour();
    
    // Dim icon when sync is off, full brightness when on
    if (!syncEnabled)
        iconColour = iconColour.withAlpha (0.4f);
    
    // Brighten on hover
    if (isHovered)
        iconColour = iconColour.brighter (0.3f);
    
    float textHeight = baseSyncTextHeight * scaleFactor;
    float iconSize = baseIconSize * scaleFactor;
    float gap = baseGap * scaleFactor;
    float fontSize = baseFontSize * scaleFactor;
    float cornerRadius = baseCornerRadius * scaleFactor;
    
    // Draw subtle background on hover
    if (isHovered)
    {
        g.setColour (iconColour.withAlpha (0.15f));
        g.fillRoundedRectangle (bounds, cornerRadius);
    }
    
    // VERTICAL LAYOUT: "SYNC" text on top, clock icon below
    
    // Draw "SYNC" text at the top (always accent color, doesn't change on toggle)
    auto textBounds = bounds.removeFromTop (textHeight);
    g.setColour (accentColour.withAlpha (syncEnabled ? 1.0f : 0.5f));
    g.setFont (juce::FontOptions (fontSize).withStyle ("Bold"));
    g.drawText ("SYNC", textBounds, juce::Justification::centred, false);
    
    // Gap
    bounds.removeFromTop (gap);
    
    // Icon bounds (remaining space, centered)
    auto iconBounds = bounds.withSizeKeepingCentre (iconSize, iconSize);
    
    // Draw the clock icon (outline when off, filled when on)
    drawClockIcon (g, iconBounds, iconColour, syncEnabled);
}

void SyncToggleControl::drawClockIcon (juce::Graphics& g, juce::Rectangle<float> bounds, 
                                        juce::Colour colour, bool filled)
{
    // Clock icon based on the provided SVGs
    // OFF (outline): Circle with clock hands (stroke only)
    // ON (filled): Filled circle with clock hands cutout
    
    g.setColour (colour);
    
    float cx = bounds.getCentreX();
    float cy = bounds.getCentreY();
    float size = juce::jmin (bounds.getWidth(), bounds.getHeight());
    float radius = size * 0.45f;
    float lineWidth = size * 0.08f;
    
    if (filled)
    {
        // Filled clock - draw filled circle then cut out the hands
        juce::Path circlePath;
        circlePath.addEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
        g.fillPath (circlePath);
        
        // Draw clock hands in the background color (to "cut out")
        // Since we can't actually cut out, we'll draw the hands in a contrasting color
        // For dark backgrounds, use the background color
        g.setColour (isAboveThumb ? juce::Colour (0xff1a1a1a) : juce::Colours::white);
        
        // Vertical line (12 o'clock to center)
        float handLength = radius * 0.55f;
        g.drawLine (cx, cy - handLength, cx, cy, lineWidth * 1.2f);
        
        // Horizontal line (center to 3 o'clock)
        float hourHandLength = radius * 0.4f;
        g.drawLine (cx, cy, cx + hourHandLength, cy, lineWidth * 1.2f);
    }
    else
    {
        // Outline clock - stroke circle and hands
        g.drawEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, lineWidth);
        
        // Vertical line (12 o'clock to center)
        float handLength = radius * 0.55f;
        g.drawLine (cx, cy - handLength, cx, cy, lineWidth);
        
        // Horizontal line (center to 3 o'clock)
        float hourHandLength = radius * 0.4f;
        g.drawLine (cx, cy, cx + hourHandLength, cy, lineWidth);
    }
}

