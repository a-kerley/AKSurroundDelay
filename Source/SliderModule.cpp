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
    // Check for Cmd+click or Alt+click reset FIRST (before JUCE handles the drag)
    // This ensures reset works when clicking directly on the slider track/thumb
    if (event.mods.isCommandDown() || event.mods.isAltDown())
    {
        if (parentModule != nullptr)
            parentModule->handleResetToDefault();
        return;  // Don't start drag
    }
    
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
// Returns BASE dimensions and asset info for each fader style (at 1.0x scale).
// These values are then multiplied by the scaleFactor when applied.
//
// SPRITESHEET NOTES:
// - All PNGs are rendered at 4x resolution for crisp Retina display
// - spritesheetFrameWidth/Height are the 4x values from the PNG
// - Divide by 4 to get display size (which should match trackWidth/trackHeight)
// - Total PNG height = spritesheetTotalFrames × spritesheetFrameHeight
//
// Example for Fader_38x170:
//   Track: 38×170 display pixels (at 1.0x scale)
//   PNG: 152×680 per frame (4x), 170 frames stacked = 152×115600 total
//==============================================================================
FaderStyleInfo SliderModule::getBaseStyleInfo (FaderStyle style)
{
    // All dimensional values are BASE (unscaled) - will be multiplied by scaleFactor
    // Spritesheet values are at 4x resolution (divide by 4 for display size)
    
    switch (style)
    {
        //======================================================================
        // STANDARD VERTICAL FADER (38×170)
        // The default/primary fader size - tall with good travel range
        //======================================================================
        case FaderStyle::Fader_38x170:
            return {
                // Track dimensions
                .trackWidth         = 38.0f,
                .trackHeight        = 170.0f,
                // Thumb dimensions  
                .thumbWidth         = 34.0f,
                .thumbHeight        = 13.0f,
                .thumbInset         = 2.8f,
                .trackYOffset       = -1.6f,
                // Spritesheet (4x resolution)
                .spritesheetTotalFrames  = 170,
                .spritesheetFrameWidth   = 152,   // 38 × 4
                .spritesheetFrameHeight  = 680,   // 170 × 4
                // Asset location
                .folderName         = "Fader 38 x 170",
                .isHorizontal       = false,
                // Text editor
                .textEditorWidth    = 38.0f,
                .textEditorPadding  = 4.0f,
                // Typography
                .valueLabelFontSize = 9.0f
            };
            
        //======================================================================
        // MEDIUM VERTICAL FADER (32×129)
        // Slightly smaller - good for dense layouts
        //======================================================================
        case FaderStyle::Fader_32x129:
            return {
                .trackWidth         = 32.0f,
                .trackHeight        = 129.0f,
                .thumbWidth         = 28.0f,
                .thumbHeight        = 13.0f,
                .thumbInset         = 2.8f,
                .trackYOffset       = -1.6f,
                .spritesheetTotalFrames  = 129,
                .spritesheetFrameWidth   = 128,   // 32 × 4
                .spritesheetFrameHeight  = 516,   // 129 × 4
                .folderName         = "Fader 32 x 129",
                .isHorizontal       = false,
                .textEditorWidth    = 32.0f,
                .textEditorPadding  = 3.0f,
                .valueLabelFontSize = 9.0f
            };
            
        //======================================================================
        // MEDIUM VERTICAL with FRONT/BACK labels (32×129)
        // Same as above but with directional labels in the SVG
        //======================================================================
        case FaderStyle::Fader_32x129_FrontBack:
            return {
                .trackWidth         = 32.0f,
                .trackHeight        = 129.0f,
                .thumbWidth         = 28.0f,
                .thumbHeight        = 13.0f,
                .thumbInset         = 2.8f,
                .trackYOffset       = -1.6f,
                .spritesheetTotalFrames  = 129,
                .spritesheetFrameWidth   = 128,
                .spritesheetFrameHeight  = 516,
                .folderName         = "Fader 32 x 129 Front-Back",
                .isHorizontal       = false,
                .textEditorWidth    = 32.0f,
                .textEditorPadding  = 3.0f,
                .valueLabelFontSize = 9.0f
            };
            
        //======================================================================
        // HORIZONTAL FADER with LEFT/RIGHT labels (28×84)
        // Used for pan controls - L/R labels in the SVG
        // Note: trackHeight is the travel dimension (horizontal), trackWidth is short
        //======================================================================
        case FaderStyle::Fader_28x84_HorizontalLeftRight:
            return {
                .trackWidth         = 28.0f,   // Short dimension (vertical)
                .trackHeight        = 84.0f,   // Travel dimension (horizontal)
                .thumbWidth         = 13.0f,
                .thumbHeight        = 24.0f,
                .thumbInset         = 2.5f,
                .trackYOffset       = 0.0f,
                .spritesheetTotalFrames  = 84,
                .spritesheetFrameWidth   = 336,  // 84 × 4 (travel direction)
                .spritesheetFrameHeight  = 112,  // 28 × 4 (short direction)
                .folderName         = "Fader 28 x 84 Horizontal Left-Right",
                .isHorizontal       = true,
                .textEditorWidth    = 40.0f,
                .textEditorPadding  = 4.0f,
                .valueLabelFontSize = 9.0f
            };
            
        //======================================================================
        // SLIM VERTICAL FADER (22×170)
        // Narrow but tall - for tight horizontal spacing with full travel
        //======================================================================
        case FaderStyle::Fader_22x170:
            return {
                .trackWidth         = 22.0f,
                .trackHeight        = 170.0f,
                .thumbWidth         = 18.0f,
                .thumbHeight        = 13.0f,
                .thumbInset         = 2.8f,
                .trackYOffset       = -1.6f,
                .spritesheetTotalFrames  = 170,
                .spritesheetFrameWidth   = 88,   // 22 × 4
                .spritesheetFrameHeight  = 680,  // 170 × 4
                .folderName         = "Fader 22 x 170",
                .isHorizontal       = false,
                .textEditorWidth    = 26.0f,
                .textEditorPadding  = 2.0f,
                .valueLabelFontSize = 8.0f       // Smaller font for slim fader
            };
            
        //======================================================================
        // SMALL VERTICAL FADER (22×79)
        // Compact - for minimal UI elements or secondary controls
        //======================================================================
        case FaderStyle::Fader_22x79:
            return {
                .trackWidth         = 22.0f,
                .trackHeight        = 79.0f,
                .thumbWidth         = 18.0f,
                .thumbHeight        = 13.0f,
                .thumbInset         = 2.8f,
                .trackYOffset       = -1.6f,
                .spritesheetTotalFrames  = 79,
                .spritesheetFrameWidth   = 88,   // 22 × 4
                .spritesheetFrameHeight  = 316,  // 79 × 4
                .folderName         = "Fader 22 x 79",
                .isHorizontal       = false,
                .textEditorWidth    = 26.0f,
                .textEditorPadding  = 2.0f,
                .valueLabelFontSize = 8.0f       // Smaller font for compact fader
            };
            
        //======================================================================
        // DEFAULT FALLBACK
        //======================================================================
        default:
            return {
                .trackWidth         = 38.0f,
                .trackHeight        = 170.0f,
                .thumbWidth         = 34.0f,
                .thumbHeight        = 13.0f,
                .thumbInset         = 2.8f,
                .trackYOffset       = -1.6f,
                .spritesheetTotalFrames  = 170,
                .spritesheetFrameWidth   = 152,
                .spritesheetFrameHeight  = 680,
                .folderName         = "Fader 38 x 170",
                .isHorizontal       = false,
                .textEditorWidth    = 38.0f,
                .textEditorPadding  = 4.0f,
                .valueLabelFontSize = 9.0f
            };
    }
}

// Apply scale factor to base style info
FaderStyleInfo SliderModule::getStyleInfo (FaderStyle style)
{
    // Get base (unscaled) info - this is now just a passthrough that applies default 1.0 scale
    // The actual scaling happens in setScaleFactor() which updates styleInfo
    return getBaseStyleInfo (style);
}

void SliderModule::setScaleFactor (float scale)
{
    scale = juce::jlimit (1.0f, 3.0f, scale);
    if (std::abs (scale - currentScaleFactor) < 0.01f)
        return;  // No significant change
    
    currentScaleFactor = scale;
    
    // Recalculate styleInfo with new scale factor
    auto baseInfo = getBaseStyleInfo (faderStyle);
    styleInfo.trackWidth = baseInfo.trackWidth * scale;
    styleInfo.trackHeight = baseInfo.trackHeight * scale;
    styleInfo.thumbWidth = baseInfo.thumbWidth * scale;
    styleInfo.thumbHeight = baseInfo.thumbHeight * scale;
    styleInfo.thumbInset = baseInfo.thumbInset * scale;
    styleInfo.trackYOffset = baseInfo.trackYOffset * scale;
    styleInfo.textEditorWidth = baseInfo.textEditorWidth * scale;
    styleInfo.textEditorPadding = baseInfo.textEditorPadding * scale;
    styleInfo.valueLabelFontSize = baseInfo.valueLabelFontSize * scale;
    // Non-scaled fields remain the same
    styleInfo.spritesheetTotalFrames = baseInfo.spritesheetTotalFrames;
    styleInfo.spritesheetFrameWidth = baseInfo.spritesheetFrameWidth;
    styleInfo.spritesheetFrameHeight = baseInfo.spritesheetFrameHeight;
    styleInfo.folderName = baseInfo.folderName;
    styleInfo.isHorizontal = baseInfo.isHorizontal;
    
    // Update label font
    nameLabel.setFont (juce::FontOptions (labelFontSize()));
    
    // Trigger relayout
    resized();
    repaint();
}

SliderModule::SliderModule (const juce::String& labelText, FaderStyle style)
    : faderStyle (style), styleInfo (getBaseStyleInfo (style))
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
    
    // Normalize drag sensitivity across all fader sizes
    // By setting a fixed pixel distance (200px) for full range travel,
    // all faders feel the same regardless of their actual height
    // This gives a consistent, comfortable drag feel
    slider.setMouseDragSensitivity (200);
    
    addAndMakeVisible (slider);
    
    // Setup name label (below slider)
    parameterName = labelText;
    if (labelText.isNotEmpty())
    {
        nameLabel.setText (labelText, juce::dontSendNotification);
        nameLabel.setFont (juce::FontOptions (labelFontSize()));  // Use scaled font size
        nameLabel.setJustificationType (juce::Justification::centred);  // Center horizontally and vertically
        nameLabel.setColour (juce::Label::textColourId, labelTextColour);
        nameLabel.setMinimumHorizontalScale (1.0f);  // Never squash text - let it overflow if needed
        nameLabel.setInterceptsMouseClicks (false, false);  // Allow clicks to pass through
        nameLabel.setPaintingIsUnclipped (true);  // Allow text to paint outside bounds if needed
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

void SliderModule::setLabelText (const juce::AttributedString& attributedText)
{
    // Store plain text version for parameter name
    parameterName = attributedText.getText();
    
    // Store the attributed string for custom painting
    // Ensure word wrap is disabled for single-line labels
    attributedLabel = attributedText;
    attributedLabel.setWordWrap (juce::AttributedString::none);
    useAttributedLabel = true;
    
    // Hide the standard label - we'll draw the AttributedString ourselves
    nameLabel.setVisible (false);
    
    repaint();
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
    
    // Draw AttributedString label if using rich text mode
    if (useAttributedLabel)
    {
        // Calculate label bounds (same logic as resized())
        auto bounds = getLocalBounds();
        bounds.removeFromTop ((int)componentPaddingTop());
        bounds.removeFromBottom ((int)componentPaddingBottom());
        auto labelBounds = bounds.removeFromBottom ((int)labelHeight());
        
        // Create a TextLayout from the AttributedString
        // Use a very large width to prevent any word wrapping - single line only
        juce::TextLayout layout;
        layout.createLayout (attributedLabel, 10000.0f);  // Large width = no wrapping
        
        // Draw centered in the label bounds
        float textWidth = layout.getWidth();
        float textHeight = layout.getHeight();
        float xOffset = ((float)labelBounds.getWidth() - textWidth) * 0.5f;
        float yOffset = ((float)labelBounds.getHeight() - textHeight) * 0.5f;
        layout.draw (g, juce::Rectangle<float> ((float)labelBounds.getX() + xOffset, 
                                                  (float)labelBounds.getY() + yOffset,
                                                  textWidth, textHeight));
    }
}

void SliderModule::resized()
{
    auto bounds = getLocalBounds();
    
    // Remove component padding from top and bottom only
    bounds.removeFromTop ((int)componentPaddingTop());
    bounds.removeFromBottom ((int)componentPaddingBottom());
    
    // Calculate label bounds - use a very wide area centered on the component
    // to prevent any text wrapping. The label has setPaintingIsUnclipped(true)
    // so it can paint beyond the component bounds if needed.
    auto labelY = bounds.getBottom() - (int)labelHeight();
    int labelWidth = 1000;  // Very wide to prevent wrapping
    int labelX = (getWidth() - labelWidth) / 2;  // Center on component
    nameLabel.setBounds (labelX, labelY, labelWidth, (int)labelHeight());
    
    bounds.removeFromBottom ((int)labelHeight());
    bounds.removeFromBottom ((int)labelSpacing());  // Add spacing gap
    
    // Now remove horizontal padding for the slider area
    bounds.removeFromLeft ((int)componentPaddingLeft());
    bounds.removeFromRight ((int)componentPaddingRight());
    
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
// MOUSE EVENTS - Double-click to edit, Cmd/Alt-click to reset
//==============================================================================

void SliderModule::handleResetToDefault()
{
    // Reset slider to its default value
    // Uses the double-click return value if set, otherwise defaults to 0.5
    if (attachment != nullptr)
    {
        // Use the slider's double-click return value as the default
        slider.setValue (slider.getDoubleClickReturnValue(), juce::sendNotificationSync);
    }
    else
    {
        slider.setValue (0.5, juce::sendNotificationSync);
    }
}

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
    
    // Cmd+click (Mac) or Alt/Option+click resets to default value
    // This handles clicks on the SliderModule background (outside slider bounds)
    // Clicks directly on the slider are handled by SliderModuleSlider::mouseDown
    if (event.mods.isCommandDown() || event.mods.isAltDown())
    {
        handleResetToDefault();
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
        valueTextEditor->setFont (juce::FontOptions (valueFontSize()));
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
    float editorHeight = valueFontSize() + 10.0f;
    
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
        float textHeight = valueFontSize() + 4.0f;
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
