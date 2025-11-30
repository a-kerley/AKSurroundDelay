#include "SVGComponents.h"

//==============================================================================
// SVGRotaryKnob Implementation
//==============================================================================

SVGRotaryKnob::SVGRotaryKnob (const juce::String& labelText, juce::Colour accentColor)
    : label (labelText), colour (accentColor)
{
    // Load SVG from file
    auto knobFile = juce::File (__FILE__).getParentDirectory()
                                         .getParentDirectory()
                                         .getChildFile ("assets")
                                         .getChildFile ("Knob Big.svg");
    
    if (knobFile.existsAsFile())
    {
        knobSVG = juce::Drawable::createFromSVGFile (knobFile);
    }
    
    // Configure slider
    setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    setRotaryParameters (minAngle, maxAngle, true);
    
    // Make it square (SVG is 116x116)
    setSize (116, 136); // Extra 20px for label at bottom
}

void SVGRotaryKnob::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto knobBounds = bounds.removeFromTop (116.0f); // SVG is 116x116
    
    if (knobSVG)
    {
        // Calculate rotation based on slider value
        float rotation = valueToAngle();
        
        // Save graphics state
        juce::Graphics::ScopedSaveState state (g);
        
        // Translate to center, rotate, translate back
        auto centre = knobBounds.getCentre();
        g.addTransform (juce::AffineTransform::rotation (rotation, centre.x, centre.y));
        
        // Draw the SVG
        knobSVG->drawWithin (g, knobBounds, juce::RectanglePlacement::centred, 1.0f);
    }
    else
    {
        // Fallback: draw simple circle if SVG failed to load
        g.setColour (juce::Colour (0xff303030));
        g.fillEllipse (knobBounds.reduced (10.0f));
        
        g.setColour (colour);
        g.drawEllipse (knobBounds.reduced (10.0f), 3.0f);
        
        // Draw pointer
        auto centre = knobBounds.getCentre();
        float rotation = valueToAngle();
        float pointerLength = knobBounds.getWidth() * 0.35f;
        
        juce::Point<float> pointerEnd (
            centre.x + pointerLength * std::cos (rotation - juce::MathConstants<float>::halfPi),
            centre.y + pointerLength * std::sin (rotation - juce::MathConstants<float>::halfPi)
        );
        
        g.setColour (colour);
        g.drawLine (centre.x, centre.y, pointerEnd.x, pointerEnd.y, 3.0f);
    }
    
    // Draw label at bottom
    auto textBounds = bounds;
    
    g.setColour (colour.withAlpha (0.8f));
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.drawText (label, textBounds, juce::Justification::centred);
    
    // Draw value text
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (10.0f));
    juce::String valueText = juce::String (getValue(), 1);
    g.drawText (valueText, textBounds.reduced (0, 5), juce::Justification::centredBottom);
}

float SVGRotaryKnob::valueToAngle() const
{
    auto normValue = (getValue() - getMinimum()) / (getMaximum() - getMinimum());
    return minAngle + normValue * (maxAngle - minAngle);
}

void SVGRotaryKnob::setAccentColour (juce::Colour newColour)
{
    colour = newColour;
    repaint();
}

//==============================================================================
// SVGVerticalSlider Implementation
//==============================================================================

SVGVerticalSlider::SVGVerticalSlider (const juce::String& labelText, juce::Colour accentColor)
    : label (labelText), colour (accentColor)
{
    // Load SVG from file
    auto sliderFile = juce::File (__FILE__).getParentDirectory()
                                           .getParentDirectory()
                                           .getChildFile ("assets")
                                           .getChildFile ("Vertical Controller.svg");
    
    if (sliderFile.existsAsFile())
    {
        sliderBackgroundSVG = juce::Drawable::createFromSVGFile (sliderFile);
    }
    
    // Configure slider
    setSliderStyle (juce::Slider::LinearVertical);
    setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    
    // Match SVG dimensions (44x427)
    setSize (44, 427);
}

void SVGVerticalSlider::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Draw dark background
    g.setColour (juce::Colour (0xff141414));
    g.fillRoundedRectangle (bounds.reduced (15.0f, 30.0f), 2.0f);
    
    // Calculate thumb position based on slider value
    float thumbY = juce::jmap (static_cast<float> (getValue()), 
                               static_cast<float> (getMinimum()),
                               static_cast<float> (getMaximum()),
                               bounds.getHeight() - 50.0f,  // Bottom position
                               30.0f);  // Top position
    
    // Draw green track from thumb to bottom
    juce::Rectangle<float> trackBounds (bounds.getCentreX() - 3.0f, thumbY, 6.0f, bounds.getHeight() - thumbY - 30.0f);
    g.setColour (colour);
    g.fillRoundedRectangle (trackBounds, 2.0f);
    
    // Draw thumb (the fader handle)
    juce::Rectangle<float> thumbBounds (bounds.getCentreX() - 10.0f, thumbY - 5.0f, 20.0f, 10.0f);
    
    g.setColour (juce::Colour (0xff141414));
    g.fillRoundedRectangle (thumbBounds, 1.5f);
    
    g.setColour (colour);
    g.drawRoundedRectangle (thumbBounds, 1.5f, 1.5f);
    
    // Draw inner line indicator on thumb
    g.fillRoundedRectangle (thumbBounds.reduced (5.0f, 3.0f).withWidth (10.0f), 1.0f);
    
    // Draw label at bottom
    g.setColour (juce::Colour (0xff8D8D8D));
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.drawText (label, bounds.removeFromBottom (25.0f), juce::Justification::centred);
    
    // Draw value
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (10.0f));
    juce::String valueText = juce::String (getValue(), 1);
    g.drawText (valueText, bounds.removeFromBottom (15.0f), juce::Justification::centred);
}

void SVGVerticalSlider::resized()
{
    // Nothing to do - fixed size based on SVG
}
