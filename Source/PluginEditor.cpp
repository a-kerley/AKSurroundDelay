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
    mixSlider.setShowDebugBorder (true);  // Enable debug border
    addAndMakeVisible (mixSlider);
    
    // Setup hue slider (for testing color system)
    hueSlider.getSlider().setLookAndFeel (&customLookAndFeel);
    hueSlider.attachToParameter (p.getParameters(), "hue");
    hueSlider.setInterval (1.0);  // Discrete steps: 0, 1, 2, ..., 9 (10 colors)
    hueSlider.getSlider().onValueChange = [this]() { updateSliderColors(); };
    hueSlider.setShowDebugBorder (true);  // Enable debug border
    addAndMakeVisible (hueSlider);
    
    // Initialize slider colors based on default hue value
    updateSliderColors();
    
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
    // Clean dark background
    g.fillAll (juce::Colour (0xff1a1a1a)); /* #1a1a1a */
    
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
    
    // Use SliderModule's ideal size calculations
    int sliderWidth = SliderModule::getIdealWidth();
    int sliderHeight = SliderModule::getIdealHeight();
    
    // Position sliders side by side at their ideal sizes
    // Mix slider on left, Hue slider on right
    mixSlider.setBounds (centerX - 100, centerY - sliderHeight/2, sliderWidth, sliderHeight);
    hueSlider.setBounds (centerX + 20, centerY - sliderHeight/2, sliderWidth, sliderHeight);
}

void TapMatrixAudioProcessorEditor::updateSliderColors()
{
    // Safety check - ensure slider is initialized
    auto value = hueSlider.getSlider().getValue();
    if (!std::isfinite (value))
        return;
    
    // Define the 10 palette colors
    static const juce::Array<juce::Colour> paletteColors = {
        juce::Colour (0xff3d3a5c), /* #3d3a5c */
        juce::Colour (0xff2b5876), /* #2b5876 */
        juce::Colour (0xff2a6f7f), /* #2a6f7f */
        juce::Colour (0xff32987e), /* #32987e */
        juce::Colour (0xff54c181), /* #54c181 */
        juce::Colour (0xff70b861), /* #70b861 */
        juce::Colour (0xff9cae4d), /* #9cae4d */
        juce::Colour (0xffc1a03e), /* #c1a03e */
        juce::Colour (0xffc78441), /* #c78441 */
        juce::Colour (0xffb76d3a)  /* #b76d3a */
    };
    
    // Define matching text colors for each palette color
    static const juce::Array<juce::Colour> textColors = {
        juce::Colour (0xffd0d0d0), /* #d0d0d0 */ // Light grey for dark purple
        juce::Colour (0xffc8c8c8), /* #c8c8c8 */ // Light grey for dark blue
        juce::Colour (0xffc0c0c0), /* #c0c0c0 */ // Medium-light grey for teal
        juce::Colour (0xff1e1e1e), /* #1e1e1e */ // Medium grey for sea green
        juce::Colour (0xff1e1e1e), /* #1e1e1e */ // Medium-dark grey for mint
        juce::Colour (0xff1e1e1e), /* #1e1e1e */ // Medium-dark grey for lime
        juce::Colour (0xff1e1e1e), /* #1e1e1e */ // Dark grey for yellow-green
        juce::Colour (0xff1e1e1e), /* #1e1e1e */ // Darker grey for gold
        juce::Colour (0xff1e1e1e), /* #1e1e1e */ // Very dark grey for orange
        juce::Colour (0xff1e1e1e)  /* #1e1e1e */ // Very dark grey for rust
    };
    
    // Get the selected color index (0-9)
    int colorIndex = juce::jlimit (0, 9, (int)value);
    auto colour = paletteColors[colorIndex];
    auto textColour = textColors[colorIndex];
    
    DBG ("updateSliderColors called: index=" + juce::String(colorIndex) + 
         " color=" + colour.toDisplayString (true));
    
    // Apply to both sliders
    mixSlider.setAccentColour (colour);
    mixSlider.setValueTextColour (textColour);
    hueSlider.setAccentColour (colour);
    hueSlider.setValueTextColour (textColour);
    
    // Force repaint of sliders
    mixSlider.repaint();
    hueSlider.repaint();
    repaint();
}
