#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TapMatrixAudioProcessorEditor::TapMatrixAudioProcessorEditor (TapMatrixAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set custom LookAndFeel
    setLookAndFeel (&customLookAndFeel);
    
    // Setup mix slider
    mixSlider.getSlider().setLookAndFeel (&customLookAndFeel);
    mixSlider.attachToParameter (p.getParameters(), "mix");
    addAndMakeVisible (mixSlider);
    
    // Setup hue slider (for testing color system)
    hueSlider.getSlider().setLookAndFeel (&customLookAndFeel);
    hueSlider.getSlider().setRange (0.0, 360.0, 1.0);  // Match parameter range
    hueSlider.attachToParameter (p.getParameters(), "hue");
    hueSlider.getSlider().onValueChange = [this]() { updateSliderColors(); };
    addAndMakeVisible (hueSlider);
    
    // Set plugin window size (1100 × 700)
    setSize (1100, 700);
}

TapMatrixAudioProcessorEditor::~TapMatrixAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
void TapMatrixAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Clean dark background (AARRGGBB format)
    g.fillAll (juce::Colour (0xff1a1a1a));
    
    // Draw center crosshair for layout reference
    g.setColour (juce::Colours::grey.withAlpha (0.3f));
    auto bounds = getLocalBounds();
    g.drawLine (bounds.getCentreX(), 0, bounds.getCentreX(), bounds.getHeight(), 1.0f);
    g.drawLine (0, bounds.getCentreY(), bounds.getWidth(), bounds.getCentreY(), 1.0f);
}

void TapMatrixAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    // Center the sliders
    int centerX = bounds.getCentreX();
    int centerY = bounds.getCentreY();
    
    // Position sliders side by side (each 80px wide + 210px tall)
    // Mix slider on left, Hue slider on right
    mixSlider.setBounds (centerX - 100, centerY - 105, 80, 210);
    hueSlider.setBounds (centerX + 20, centerY - 105, 80, 210);
}

void TapMatrixAudioProcessorEditor::updateSliderColors()
{
    // Safety check - ensure slider is initialized
    auto value = hueSlider.getSlider().getValue();
    if (!std::isfinite (value))
        return;
    
    // Get hue value (0-360) and normalize to 0-1
    float hue = value / 360.0f;
    
    // Create color from HSV (hue, full saturation, full brightness)
    auto colour = juce::Colour::fromHSV (hue, 0.8f, 0.9f, 1.0f);
    
    DBG ("updateSliderColors called: hue=" + juce::String(value) + 
         " color=" + colour.toDisplayString (true));
    
    // Apply to both sliders
    mixSlider.setAccentColour (colour);
    hueSlider.setAccentColour (colour);
    
    // Force repaint of sliders
    mixSlider.repaint();
    hueSlider.repaint();
    repaint();
}
