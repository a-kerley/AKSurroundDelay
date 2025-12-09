#include "ViewPresetSelector.h"

//==============================================================================
ViewPresetSelector::ViewPresetSelector()
{
    // Start animation timer (60 FPS)
    startTimerHz (60);
}

ViewPresetSelector::~ViewPresetSelector()
{
    stopTimer();
}

//==============================================================================
void ViewPresetSelector::setCurrentPreset (SurroundStageView::ViewPreset preset)
{
    // Find the index for this preset
    for (int i = 0; i < numPresets; ++i)
    {
        if (presets[i] == preset)
        {
            currentIndex = i;
            targetPosition = (float)i;
            isCustom = false;
            repaint();
            return;
        }
    }
    
    // If preset is Custom, just dim the current selection
    if (preset == SurroundStageView::ViewPreset::Custom)
    {
        isCustom = true;
        repaint();
    }
}

void ViewPresetSelector::setCustomState (bool custom)
{
    if (isCustom != custom)
    {
        isCustom = custom;
        repaint();
    }
}

void ViewPresetSelector::setScaleFactor (float scale)
{
    if (currentScaleFactor != scale)
    {
        currentScaleFactor = scale;
        repaint();
    }
}

//==============================================================================
void ViewPresetSelector::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Draw background with border
    g.setColour (backgroundColour);
    g.fillRoundedRectangle (bounds, cornerRadius);
    
    g.setColour (borderColour);
    g.drawRoundedRectangle (bounds.reduced (borderWidth * 0.5f), cornerRadius, borderWidth);
    
    // Draw the animated pill (highlight for selected segment)
    auto pillBounds = getPillBounds();
    g.setColour (isCustom ? pillDimmedColour : pillColour);
    g.fillRoundedRectangle (pillBounds, cornerRadius - pillPadding);
    
    // Draw segment labels
    g.setFont (juce::FontOptions (baseFontSize * currentScaleFactor).withStyle ("Bold"));
    
    for (int i = 0; i < numPresets; ++i)
    {
        auto segmentBounds = getSegmentBounds (i);
        
        // Text color: white if this is the selected segment, dimmed otherwise
        bool isSelected = (i == currentIndex);
        if (isCustom)
            g.setColour (textDimmedColour);
        else
            g.setColour (isSelected ? textColour : textDimmedColour);
        
        g.drawText (presetLabels[i], segmentBounds, juce::Justification::centred);
    }
}

void ViewPresetSelector::resized()
{
    // Nothing special needed - bounds are calculated dynamically
}

void ViewPresetSelector::mouseDown (const juce::MouseEvent& event)
{
    // Determine which segment was clicked
    for (int i = 0; i < numPresets; ++i)
    {
        if (getSegmentBounds (i).contains (event.position))
        {
            currentIndex = i;
            targetPosition = (float)i;
            isCustom = false;
            
            if (onPresetSelected)
                onPresetSelected (presets[i]);
            
            repaint();
            return;
        }
    }
}

//==============================================================================
void ViewPresetSelector::timerCallback()
{
    // Animate pill position toward target
    if (std::abs (pillPosition - targetPosition) > 0.001f)
    {
        // Ease-out animation
        pillPosition += (targetPosition - pillPosition) * animationSpeed;
        repaint();
    }
    else if (pillPosition != targetPosition)
    {
        pillPosition = targetPosition;
        repaint();
    }
}

juce::Rectangle<float> ViewPresetSelector::getSegmentBounds (int index) const
{
    auto bounds = getLocalBounds().toFloat();
    float segmentWidth = bounds.getWidth() / numPresets;
    
    return juce::Rectangle<float> (
        bounds.getX() + index * segmentWidth,
        bounds.getY(),
        segmentWidth,
        bounds.getHeight()
    );
}

juce::Rectangle<float> ViewPresetSelector::getPillBounds() const
{
    auto bounds = getLocalBounds().toFloat().reduced (pillPadding);
    float segmentWidth = bounds.getWidth() / numPresets;
    
    return juce::Rectangle<float> (
        bounds.getX() + pillPosition * segmentWidth,
        bounds.getY(),
        segmentWidth,
        bounds.getHeight()
    );
}
