#include "SliderModule.h"
#include "ColorPalette.h"

//==============================================================================
// CUSTOM SLIDER - Forwards double-clicks to parent SliderModule
//==============================================================================
void SliderModuleSlider::mouseDoubleClick (const juce::MouseEvent& event)
{
    // Don't call base class - we handle double-click ourselves
    // Forward to parent SliderModule to show text editor
    if (parentModule != nullptr)
        parentModule->mouseDoubleClick (event);
}

void SliderModuleSlider::mouseDown (const juce::MouseEvent& event)
{
    // If text editor is open, dismiss it first
    if (parentModule != nullptr && parentModule->isTextEditorActive())
    {
        parentModule->dismissTextEditor (true);  // Commit value
        return;  // Don't start dragging on this click
    }
    
    // Otherwise, proceed with normal slider behavior
    juce::Slider::mouseDown (event);
}

//==============================================================================
// STATIC MEMBER INITIALIZATION
// Shared sprite caches - loaded once per style, shared across all instances
//==============================================================================
std::map<FaderStyle, juce::Image> SliderModule::fillBarImages;        // Base spritesheet per style
std::map<FaderStyle, bool> SliderModule::fillBarImagesLoaded;         // Load status flags
std::map<std::pair<FaderStyle, int>, juce::Image> SliderModule::colorVariants;  // Pre-tinted variants (style, colorIndex)
std::map<FaderStyle, bool> SliderModule::colorVariantsLoaded;         // Color variant load status

//==============================================================================
// FADER STYLE INFO LOOKUP
//
// Returns dimensions and asset info for each fader style.
// All dimensions are multiplied by uiScale for consistent scaling.
//
// SPRITESHEET NOTES:
// - All PNGs are rendered at 4x resolution for crisp Retina display
// - spritesheetFrameWidth/Height are the 4x values from the PNG
// - Divide by 4 to get display size (which should match trackWidth/trackHeight)
// - Total PNG height = spritesheetTotalFrames × spritesheetFrameHeight
//
// Example for Fader_38x170:
//   Track: 38×170 display pixels
//   PNG: 152×680 per frame (4x), 170 frames stacked = 152×115600 total
//==============================================================================
FaderStyleInfo SliderModule::getStyleInfo (FaderStyle style)
{
    constexpr float s = uiScale;  // Shorthand for scaling
    
    // Return format: { trackW, trackH, thumbW, thumbH, thumbInset, frames, frameW@4x, frameH@4x, folder, isHoriz }
    switch (style)
    {
        case FaderStyle::Fader_38x170:
            // STANDARD VERTICAL FADER (default size)
            // Display size: 38×170 pixels
            // Spritesheet: 152×680 per frame (4x), 170 frames total (one per pixel of travel)
            // PNG total: 152 × 115600 (170 × 680)
            return { 38.0f * s,   // trackWidth
                     170.0f * s,  // trackHeight
                     34.0f * s,   // thumbWidth (SVG thumb, not used for vertical)
                     13.0f * s,   // thumbHeight (affects value text height)
                     2.5f * s,    // thumbInset - keeps value text away from track edges
                     -2.0f * s,   // trackYOffset - nudge track up(-) or down(+)
                     170,         // frames (= trackHeight in pixels, one frame per position)
                     152, 680,    // frameWidth, frameHeight at 4x resolution
                     "Fader 38 x 170", false,
                     38.0f * s,   // textEditorWidth
                     4.0f * s };  // textEditorPadding
            
        case FaderStyle::Fader_32x129:
            // MEDIUM VERTICAL FADER
            // Display size: 32×129 pixels
            // Spritesheet: 128×516 per frame (4x), 129 frames total
            // PNG total: 128 × 66564 (129 × 516)
            return { 32.0f * s,   // trackWidth
                     129.0f * s,  // trackHeight
                     28.0f * s,   // thumbWidth (SVG thumb, not used for vertical)
                     13.0f * s,   // thumbHeight (affects value text height)
                     2.5f * s,    // thumbInset - keeps value text away from track edges
                     -2.0f * s,   // trackYOffset - nudge track up(-) or down(+)
                     129,         // frames (= trackHeight in pixels)
                     128, 516,    // frameWidth, frameHeight at 4x resolution
                     "Fader 32 x 129", false,
                     32.0f * s,   // textEditorWidth
                     3.0f * s };  // textEditorPadding
            
        case FaderStyle::Fader_32x129_FrontBack:
            // MEDIUM VERTICAL with "FRONT"/"BACK" labels printed on track SVG
            // Display size: 32×129 pixels (same as Fader_32x129)
            // Spritesheet: 128×516 per frame (4x), 129 frames total
            // PNG total: 128 × 66564 (129 × 516)
            return { 32.0f * s,   // trackWidth
                     129.0f * s,  // trackHeight
                     28.0f * s,   // thumbWidth (SVG thumb, not used for vertical)
                     13.0f * s,   // thumbHeight (affects value text height)
                     2.5f * s,    // thumbInset - keeps value text away from track edges
                     -2.0f * s,   // trackYOffset - nudge track up(-) or down(+)
                     129,         // frames (= trackHeight in pixels)
                     128, 516,    // frameWidth, frameHeight at 4x resolution
                     "Fader 32 x 129 Front-Back", false,
                     32.0f * s,   // textEditorWidth
                     3.0f * s };  // textEditorPadding
            
        case FaderStyle::Fader_28x84_HorizontalLeftRight:
            // HORIZONTAL FADER with "L"/"R" labels
            // Display size: 28px tall × 84px wide (width = travel direction)
            // Note: For horizontal, trackWidth = height, trackHeight = width (travel)
            // Spritesheet: 336×112 per frame (4x), 84 frames total
            // PNG total: 336 × 9408 (84 × 112)
            return { 28.0f * s,   // trackWidth (displayed as height for horizontal)
                     84.0f * s,   // trackHeight (displayed as width, travel direction)
                     13.0f * s,   // thumbWidth
                     24.0f * s,   // thumbHeight
                     2.5f * s,    // thumbInset - keeps value text away from track edges
                     0.0f * s,    // trackXOffset - nudge track left(-) or right(+)
                     84,          // frames (= travel distance in pixels)
                     336, 112,    // frameWidth, frameHeight at 4x resolution
                     "Fader 28 x 84 Horizontal Left-Right", true,
                     40.0f * s,   // textEditorWidth (wider for horizontal)
                     4.0f * s };  // textEditorPadding
            
        case FaderStyle::Fader_22x170:
            // SLIM VERTICAL FADER (tall, narrow)
            // Display size: 22×170 pixels
            // Spritesheet: 88×680 per frame (4x), 170 frames total
            // PNG total: 88 × 115600 (170 × 680)
            return { 22.0f * s,   // trackWidth
                     170.0f * s,  // trackHeight
                     18.0f * s,   // thumbWidth (SVG thumb, not used for vertical)
                     13.0f * s,   // thumbHeight (affects value text height)
                     2.5f * s,    // thumbInset - keeps value text away from track edges
                     -2.0f * s,   // trackYOffset - nudge track up(-) or down(+)
                     170,         // frames (= trackHeight in pixels)
                     88, 680,     // frameWidth, frameHeight at 4x resolution
                     "Fader 22 x 170", false,
                     26.0f * s,   // textEditorWidth (wider than track for usability)
                     2.0f * s };  // textEditorPadding
            
        case FaderStyle::Fader_22x79:
            // SMALL VERTICAL FADER (compact)
            // Display size: 22×79 pixels
            // Spritesheet: 88×316 per frame (4x), 79 frames total
            // PNG total: 88 × 24964 (79 × 316)
            return { 22.0f * s,   // trackWidth
                     79.0f * s,   // trackHeight
                     18.0f * s,   // thumbWidth (SVG thumb, not used for vertical)
                     13.0f * s,   // thumbHeight (affects value text height)
                     2.5f * s,    // thumbInset - keeps value text away from track edges
                     -2.0f * s,   // trackYOffset - nudge track up(-) or down(+)
                     79,          // frames (= trackHeight in pixels)
                     88, 316,     // frameWidth, frameHeight at 4x resolution
                     "Fader 22 x 79", false,
                     26.0f * s,   // textEditorWidth (wider than track for usability)
                     2.0f * s };  // textEditorPadding
            
        default:
            // Fallback to standard 38×170 size
            return { 38.0f * s,   // trackWidth
                     170.0f * s,  // trackHeight
                     34.0f * s,   // thumbWidth (SVG thumb, not used for vertical)
                     13.0f * s,   // thumbHeight (affects value text height)
                     2.5f * s,    // thumbInset - keeps value text away from track edges
                     -2.0f * s,   // trackYOffset - nudge track up(-) or down(+)
                     170,         // frames (= trackHeight in pixels)
                     152, 680,    // frameWidth, frameHeight at 4x resolution
                     "Fader 38 x 170", false,
                     38.0f * s,   // textEditorWidth
                     4.0f * s };  // textEditorPadding
    }
}

SliderModule::SliderModule (const juce::String& labelText, FaderStyle style)
    : faderStyle (style), styleInfo (getStyleInfo (style))
{
    // Disable clipping so labels can extend beyond component bounds
    setInterceptsMouseClicks (true, true);
    setPaintingIsUnclipped (true);
    
    // Setup slider - use horizontal style for horizontal faders
    slider.setParentModule (this);  // Allow slider to forward double-clicks to us
    slider.setSliderStyle (styleInfo.isHorizontal ? juce::Slider::LinearHorizontal : juce::Slider::LinearVertical);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);  // No text box - value shown in thumb
    slider.setColour (juce::Slider::backgroundColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::trackColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff2a2a2a)); /* #2a2a2a */
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setRange (0.0, 1.0, 0.001); // Finer step for smoother animation (was 0.01)
    slider.setValue (0.5, juce::dontSendNotification); // Set default value
    slider.setDoubleClickReturnValue (false, 0.5);  // Disable JUCE's built-in double-click (we handle it ourselves)
    slider.setSliderSnapsToMousePosition (false);  // Use relative dragging instead of jumping to click position
    addAndMakeVisible (slider);
    
    // Setup name label (below slider)
    parameterName = labelText;
    if (labelText.isNotEmpty())
    {
        nameLabel.setText (labelText, juce::dontSendNotification);
        nameLabel.setFont (juce::FontOptions (labelFontSize));
        nameLabel.setJustificationType (juce::Justification::centredTop);  // Top-align to prevent stretching
        nameLabel.setColour (juce::Label::textColourId, labelTextColour);
        nameLabel.setMinimumHorizontalScale (1.0f);  // Don't squash text horizontally
        nameLabel.setInterceptsMouseClicks (false, false);  // Allow clicks to pass through
        addAndMakeVisible (nameLabel);
    }
    
    // Listen to slider changes to repaint fill bar
    slider.onValueChange = [this]() 
    { 
        repaint();  // Repaint to update fill bar position
    };
    
    // Load fill bar spritesheet for this style (shared by all instances of same style)
    loadFillBarForStyle();
}

SliderModule::~SliderModule()
{
}

void SliderModule::attachToParameter (juce::AudioProcessorValueTreeState& apvts,
                                      const juce::String& parameterID)
{
    // Store the parameter ID for reference
    currentParameterID = parameterID;
    
    // Create or recreate the attachment
    // This automatically detaches from any previous parameter
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, parameterID, slider);
}

void SliderModule::detachFromParameter()
{
    // Reset attachment to detach from parameter
    attachment.reset();
    currentParameterID.clear();
    slider.setValue (0.5, juce::dontSendNotification);
}

void SliderModule::setLabelText (const juce::String& text)
{
    parameterName = text;
    nameLabel.setText (text, juce::dontSendNotification);
}

void SliderModule::setUsePanDisplay (bool usePan)
{
    usePanDisplay = usePan;
    
    // For horizontal pan sliders, we need to handle the slider direction
    // The default horizontal slider has left=min, right=max which is correct for L/R pan
    // where negative values = Left, positive values = Right
    // No inversion needed - the spritesheet animation handles the visual
}

void SliderModule::loadFillBarForStyle()
{
    // Check if already loaded for this style
    if (fillBarImagesLoaded[faderStyle])
        return;
    
    auto assetsPath = juce::File (getAssetsBasePath());
    auto stylePath = assetsPath.getChildFile (styleInfo.folderName);
    auto fillBarFile = stylePath.getChildFile (styleInfo.folderName + "_sprite_sheet.png");
    
    if (fillBarFile.existsAsFile())
    {
        auto image = juce::ImageCache::getFromFile (fillBarFile);
        if (!image.isNull())
        {
            fillBarImages[faderStyle] = image;
            fillBarImagesLoaded[faderStyle] = true;
            
            // Load color variants for this style
            loadColorVariantsForStyle();
        }
    }
    else
    {
        DBG ("ERROR: Fill bar spritesheet not found: " + fillBarFile.getFullPathName());
    }
}

void SliderModule::loadColorVariantsForStyle()
{
    // Check if already loaded for this style
    if (colorVariantsLoaded[faderStyle])
        return;
    
    auto assetsPath = juce::File (getAssetsBasePath());
    auto stylePath = assetsPath.getChildFile (styleInfo.folderName);
    
    // Check if cache files exist - if not, generate them first
    bool cacheExists = true;
    for (int i = 0; i < 10; ++i)
    {
        auto variantFile = stylePath.getChildFile (styleInfo.folderName + "_color" + juce::String(i) + ".png");
        if (!variantFile.existsAsFile())
        {
            cacheExists = false;
            break;
        }
    }
    
    if (!cacheExists)
        generateColorVariantCacheForStyle();
    
    // Load each pre-cached color variant PNG file
    for (int i = 0; i < 10; ++i)
    {
        auto variantFile = stylePath.getChildFile (styleInfo.folderName + "_color" + juce::String(i) + ".png");
        
        if (variantFile.existsAsFile())
        {
            auto variantImage = juce::ImageCache::getFromFile (variantFile);
            if (!variantImage.isNull())
                colorVariants[{faderStyle, i}] = variantImage;
        }
    }
    
    colorVariantsLoaded[faderStyle] = true;
}

void SliderModule::generateColorVariantCacheForStyle()
{
    // Get palette colors from central definition
    auto paletteColors = ColorPalette::getBackgroundColors();
    
    // Ensure fill bar is loaded for this style
    auto it = fillBarImages.find (faderStyle);
    if (it == fillBarImages.end() || it->second.isNull())
    {
        DBG ("ERROR: Cannot generate color variants - source image not loaded for style: " + styleInfo.folderName);
        return;
    }
    
    const auto& sourceImage = it->second;
    
    auto assetsPath = juce::File (getAssetsBasePath());
    auto stylePath = assetsPath.getChildFile (styleInfo.folderName);
    
    // Generate tinted variants for each palette color and save as PNG
    for (size_t i = 0; i < paletteColors.size(); ++i)
    {
        auto colour = paletteColors[i];
        
        // Clone and tint the spritesheet
        auto tintedImage = sourceImage.createCopy();
        
        // Fast color multiplication using BitmapData
        juce::Image::BitmapData bitmapData (tintedImage, juce::Image::BitmapData::readWrite);
        
        float tintR = colour.getFloatRed();
        float tintG = colour.getFloatGreen();
        float tintB = colour.getFloatBlue();
        
        for (int y = 0; y < bitmapData.height; ++y)
        {
            for (int x = 0; x < bitmapData.width; ++x)
            {
                auto pixel = bitmapData.getPixelColour (x, y);
                auto tinted = juce::Colour::fromFloatRGBA (
                    pixel.getFloatRed() * tintR,
                    pixel.getFloatGreen() * tintG,
                    pixel.getFloatBlue() * tintB,
                    pixel.getFloatAlpha()
                );
                bitmapData.setPixelColour (x, y, tinted);
            }
        }
        
        // Save as PNG file
        auto outputFile = stylePath.getChildFile (styleInfo.folderName + "_color" + juce::String(i) + ".png");
        juce::FileOutputStream outputStream (outputFile);
        
        if (outputStream.openedOk())
        {
            juce::PNGImageFormat pngFormat;
            pngFormat.writeImageToStream (tintedImage, outputStream);
            DBG ("Generated color variant: " + outputFile.getFullPathName());
        }
    }
}

const juce::Image& SliderModule::getVariantForColor (const juce::Colour& colour) const
{
    // Get palette colors from central definition
    auto paletteColors = ColorPalette::getBackgroundColors();
    
    // Find the closest matching palette color
    int closestIndex = 0;
    float minDistance = std::numeric_limits<float>::max();
    
    for (size_t i = 0; i < paletteColors.size(); ++i)
    {
        // Simple color distance using RGB euclidean distance
        float dr = colour.getFloatRed() - paletteColors[i].getFloatRed();
        float dg = colour.getFloatGreen() - paletteColors[i].getFloatGreen();
        float db = colour.getFloatBlue() - paletteColors[i].getFloatBlue();
        float distance = dr*dr + dg*dg + db*db;
        
        if (distance < minDistance)
        {
            minDistance = distance;
            closestIndex = (int)i;
        }
    }
    
    // Look up the variant for this fader style
    auto it = colorVariants.find ({faderStyle, closestIndex});
    if (it != colorVariants.end())
        return it->second;
    
    // Fallback to original grayscale if variant not found
    auto fillIt = fillBarImages.find (faderStyle);
    if (fillIt != fillBarImages.end())
        return fillIt->second;
    
    // Ultimate fallback - return empty static image
    static juce::Image emptyImage;
    return emptyImage;
}

void SliderModule::paint (juce::Graphics& g)
{
    // Get the fill bar image for this style
    auto fillIt = fillBarImages.find (faderStyle);
    if (fillIt == fillBarImages.end() || fillIt->second.isNull())
        return;  // No spritesheet loaded for this style
    
    // Get slider bounds
    auto sliderBounds = slider.getBounds();
    
    // Calculate which frame to display based on slider value
    double sliderValue = slider.getValue();
    double minValue = slider.getMinimum();
    double maxValue = slider.getMaximum();
    
    // Normalize value to 0-1 range (works for any slider range)
    float normalizedValue = (float)((sliderValue - minValue) / (maxValue - minValue));
    
    // Safety check - clamp to valid range
    if (!std::isfinite(normalizedValue))
        return;  // Skip drawing if value is invalid
    
    normalizedValue = juce::jlimit (0.0f, 1.0f, normalizedValue);
    
    // Calculate frame index based on slider orientation
    int totalFrames = styleInfo.spritesheetTotalFrames;
    int frameIndex;
    
    if (styleInfo.isHorizontal)
    {
        // Horizontal slider: Frame 0 = left (value 0.0/min), Last frame = right (value 1.0/max)
        // For L/R pan: -1 = left, 0 = center, +1 = right
        // normalizedValue 0.0 = left, 0.5 = center, 1.0 = right
        frameIndex = juce::jlimit (0, totalFrames - 1, 
                                   juce::roundToInt (normalizedValue * (totalFrames - 1)));
    }
    else
    {
        // Vertical slider: Spritesheet is reversed - Frame 0 = full (value 1.0), Last frame = empty (value 0.0)
        frameIndex = juce::jlimit (0, totalFrames - 1, 
                                   juce::roundToInt ((1.0f - normalizedValue) * (totalFrames - 1)));
    }
    
    // Calculate source Y position in the spritesheet
    // Frames are stacked vertically from top (frame 0) to bottom (last frame)
    int srcY = frameIndex * styleInfo.spritesheetFrameHeight;
    
    // Destination bounds (scale down from 4x to 1x)
    float fillX = (float)sliderBounds.getX();
    float fillY = (float)sliderBounds.getY();
    
    // Enable high-quality resampling for smooth 4x downscaling
    g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
    
    // Get the pre-tinted variant for this slider's accent color (instant lookup!)
    const auto& spritesheetToUse = getVariantForColor (accentColour);
    
    // Draw the pre-tinted spritesheet frame
    // For horizontal sliders: display width = trackHeight (travel), display height = trackWidth (short)
    // For vertical sliders: display width = trackWidth, display height = trackHeight
    if (styleInfo.isHorizontal)
    {
        // Horizontal slider: trackHeight is the travel dimension (84), trackWidth is short dimension (28)
        // Display as: 84 wide × 28 tall
        g.drawImage (spritesheetToUse,
                    (int)fillX, (int)fillY, (int)styleInfo.trackHeight, (int)styleInfo.trackWidth,
                    0, srcY, styleInfo.spritesheetFrameWidth, styleInfo.spritesheetFrameHeight,
                    false);
    }
    else
    {
        // Vertical slider: normal orientation
        g.drawImage (spritesheetToUse,
                    (int)fillX, (int)fillY, (int)styleInfo.trackWidth, (int)styleInfo.trackHeight,
                    0, srcY, styleInfo.spritesheetFrameWidth, styleInfo.spritesheetFrameHeight,
                    false);
    }
    
    // Draw debug border if enabled
    if (showDebugBorder)
    {
        g.setColour (juce::Colours::red);
        g.drawRect (getLocalBounds(), 1);
    }
}

void SliderModule::resized()
{
    auto bounds = getLocalBounds();
    
    // Remove component padding from all sides
    bounds.removeFromTop ((int)componentPaddingTop);
    bounds.removeFromBottom ((int)componentPaddingBottom);
    bounds.removeFromLeft ((int)componentPaddingLeft);
    bounds.removeFromRight ((int)componentPaddingRight);
    
    // Layout: [Slider] [Spacing] [Name Label]
    // Name label below slider with configurable spacing
    nameLabel.setBounds (bounds.removeFromBottom ((int)labelHeight));
    bounds.removeFromBottom ((int)labelSpacing);  // Add spacing gap
    
    // Slider dimensions - for horizontal, swap width and height
    int sliderWidth, sliderHeight;
    if (styleInfo.isHorizontal)
    {
        // Horizontal slider: trackHeight is travel (wide), trackWidth is short (tall)
        sliderWidth = (int)styleInfo.trackHeight;
        sliderHeight = (int)styleInfo.trackWidth;
    }
    else
    {
        sliderWidth = (int)styleInfo.trackWidth;
        sliderHeight = (int)styleInfo.trackHeight;
    }
    
    auto sliderBounds = bounds.withSizeKeepingCentre (sliderWidth, sliderHeight);
    slider.setBounds (sliderBounds);
}

//==============================================================================
// MOUSE EVENTS - Double-click to edit, Cmd-click to reset
//==============================================================================

void SliderModule::mouseDoubleClick (const juce::MouseEvent& event)
{
    // Double-click anywhere on the slider module opens text editor for manual value entry
    if (!event.mods.isCommandDown())
    {
        showTextEditor();
    }
}

void SliderModule::mouseDown (const juce::MouseEvent& event)
{
    // If text editor is open and click is outside it, close it
    // This handles clicks both within and outside this component's bounds
    // (we listen to the top-level component when text editor is open)
    if (isEditingValue && valueTextEditor != nullptr)
    {
        // Convert click position to our coordinate space
        auto clickPos = event.getEventRelativeTo (this).getPosition();
        
        // Check if click is outside the text editor
        if (!valueTextEditor->getBounds().contains (clickPos))
        {
            hideTextEditor (true);  // Commit value when clicking outside
            return;
        }
    }
    
    // Cmd+click (Mac) or Ctrl+click (Windows/Linux) resets to default value
    if (event.mods.isCommandDown())
    {
        // Get the parameter's default value if attached, otherwise use 0.5
        if (attachment != nullptr)
        {
            // Use the slider's double-click return value as the default
            slider.setValue (slider.getDoubleClickReturnValue(), juce::sendNotificationSync);
        }
        else
        {
            slider.setValue (0.5, juce::sendNotificationSync);
        }
        return;
    }
    
    // Otherwise, pass the event to the slider
    // (This allows normal drag behavior to work)
}

//==============================================================================
// TEXT EDITOR - Manual value entry via double-click
//==============================================================================

void SliderModule::showTextEditor()
{
    if (isEditingValue)
        return;
    
    isEditingValue = true;
    
    // Create the text editor if it doesn't exist
    if (valueTextEditor == nullptr)
    {
        valueTextEditor = std::make_unique<juce::TextEditor>();
        valueTextEditor->setMultiLine (false);
        valueTextEditor->setReturnKeyStartsNewLine (false);
        valueTextEditor->setScrollbarsShown (false);
        valueTextEditor->setCaretVisible (true);
        valueTextEditor->setPopupMenuEnabled (false);
        valueTextEditor->setJustification (juce::Justification::centred);
        valueTextEditor->setFont (juce::FontOptions (valueFontSize));
        valueTextEditor->setIndents (0, 0);  // Remove internal padding for better vertical centering
        valueTextEditor->setBorder (juce::BorderSize<int> (0));  // No internal border
        valueTextEditor->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff2a2a2a)); /* #2a2a2a */
        valueTextEditor->setColour (juce::TextEditor::textColourId, juce::Colours::white);
        valueTextEditor->setSelectAllWhenFocused (true);
        
        // Set up callbacks for enter and escape keys
        valueTextEditor->onReturnKey = [this]() { textEditorReturnKeyPressed(); };
        valueTextEditor->onEscapeKey = [this]() { textEditorEscapeKeyPressed(); };
        valueTextEditor->onFocusLost = [this]() { textEditorFocusLost(); };
        
        addAndMakeVisible (*valueTextEditor);
    }
    
    // Update border color to match current accent color
    valueTextEditor->setColour (juce::TextEditor::outlineColourId, accentColour);
    valueTextEditor->setColour (juce::TextEditor::focusedOutlineColourId, accentColour);
    
    // Calculate position to match the value label position (same as CustomLookAndFeel)
    auto sliderBounds = slider.getBounds();
    double value = slider.getValue();
    double normValue = slider.valueToProportionOfLength (value);
    
    // Text editor dimensions
    // Height: font size + vertical padding for text + border space
    float editorHeight = valueFontSize + 10.0f;
    
    // Width: use per-style configured width
    float editorWidth = styleInfo.textEditorWidth;
    
    float editorX, editorY;
    
    if (styleInfo.isHorizontal)
    {
        // Horizontal slider: position at thumb X
        float displayX = sliderBounds.getX() + (sliderBounds.getWidth() - styleInfo.trackHeight) * 0.5f;
        float displayY = sliderBounds.getY() + (sliderBounds.getHeight() - styleInfo.trackWidth) * 0.5f;
        
        float halfThumb = styleInfo.thumbWidth * 0.5f;
        float travelLeft = displayX + styleInfo.thumbInset + halfThumb;
        float travelRight = displayX + styleInfo.trackHeight - styleInfo.thumbInset - halfThumb;
        float travelRange = travelRight - travelLeft;
        float thumbCenterX = travelLeft + ((float)normValue * travelRange) + styleInfo.trackYOffset;
        
        editorX = thumbCenterX - editorWidth * 0.5f;
        editorY = displayY + (styleInfo.trackWidth - editorHeight) * 0.5f;
    }
    else
    {
        // Vertical slider: position at value label Y (same calculation as CustomLookAndFeel)
        float textHeight = valueFontSize + 4.0f;
        float trackY = sliderBounds.getY() + (sliderBounds.getHeight() - styleInfo.trackHeight) * 0.5f + styleInfo.trackYOffset;
        
        float halfText = textHeight * 0.5f;
        float travelTop = trackY + styleInfo.thumbInset + halfText;
        float travelBottom = trackY + styleInfo.trackHeight - styleInfo.thumbInset - halfText;
        float travelRange = travelBottom - travelTop;
        float valueCenterY = travelTop + (1.0f - (float)normValue) * travelRange;
        
        editorX = sliderBounds.getX() + (sliderBounds.getWidth() - editorWidth) * 0.5f;
        editorY = valueCenterY - editorHeight * 0.5f;
    }
    
    valueTextEditor->setBounds ((int)editorX, (int)editorY, (int)editorWidth, (int)editorHeight);
    
    // Set the current value as text (without suffix for easier editing)
    juce::String valueText;
    
    if (valueDecimalPlaces == 0 || std::abs(value) >= 100.0)
        valueText = juce::String ((int)value);
    else
        valueText = juce::String (value, valueDecimalPlaces);
    
    valueTextEditor->setText (valueText, false);
    valueTextEditor->setVisible (true);
    valueTextEditor->grabKeyboardFocus();
    valueTextEditor->selectAll();
    
    // Add mouse listener to top-level component to catch clicks outside
    if (auto* topLevel = getTopLevelComponent())
        topLevel->addMouseListener (this, true);
}

void SliderModule::hideTextEditor (bool commitValue)
{
    if (!isEditingValue || valueTextEditor == nullptr)
        return;
    
    if (commitValue)
    {
        // Parse the entered value and apply it to the slider
        juce::String text = valueTextEditor->getText().trim();
        double newValue = text.getDoubleValue();
        
        // Clamp to slider range
        newValue = juce::jlimit (slider.getMinimum(), slider.getMaximum(), newValue);
        
        slider.setValue (newValue, juce::sendNotificationSync);
    }
    
    // Remove mouse listener from top-level component
    if (auto* topLevel = getTopLevelComponent())
        topLevel->removeMouseListener (this);
    
    valueTextEditor->setVisible (false);
    isEditingValue = false;
    repaint();
}

void SliderModule::textEditorReturnKeyPressed()
{
    hideTextEditor (true);  // Commit the value
}

void SliderModule::textEditorEscapeKeyPressed()
{
    hideTextEditor (false);  // Cancel without committing
}

void SliderModule::textEditorFocusLost()
{
    // Commit value when focus is lost (clicking elsewhere)
    hideTextEditor (true);
}
