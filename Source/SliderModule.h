#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
// Forward declaration
class SliderModule;

//==============================================================================
/**
 * Custom slider that ignores double-clicks (forwards them to parent SliderModule)
 * This prevents JUCE's built-in double-click-to-reset behavior.
 */
class SliderModuleSlider : public juce::Slider
{
public:
    SliderModuleSlider() = default;
    
    void setParentModule (SliderModule* parent) { parentModule = parent; }
    
    void mouseDoubleClick (const juce::MouseEvent& event) override;
    void mouseDown (const juce::MouseEvent& event) override;
    
private:
    SliderModule* parentModule = nullptr;
};

//==============================================================================
/**
 * FADER STYLE ENUMERATION
 * 
 * Each style corresponds to a folder in assets/ containing:
 *   - [FolderName]_frame.svg      → Track background (slot/groove)
 *   - [FolderName]_sprite_sheet.png → Animated fill bar with thumb (4x resolution, vertical strip)
 *   - [FolderName]_color0-9.png   → Pre-tinted color variants (auto-generated if missing)
 * 
 * Naming convention: Fader_[width]x[height][_variant]
 *   - Width is the track width in pixels (unscaled)
 *   - Height is the track height in pixels (unscaled)  
 *   - Variant suffix for special labeling (e.g., FrontBack, HorizontalLeftRight)
 */
enum class FaderStyle
{
    Fader_38x170,                    // Standard vertical fader - 38px wide, 170px tall (default)
    Fader_32x129,                    // Medium vertical fader - 32px wide, 129px tall
    Fader_32x129_FrontBack,          // Medium vertical with "Front"/"Back" labels printed on track
    Fader_28x84_HorizontalLeftRight, // Horizontal fader - 28px tall, 84px wide (travel direction)
    Fader_22x170,                    // Slim vertical fader - 22px wide, 170px tall
    Fader_22x79                      // Small vertical fader - 22px wide, 79px tall
};

//==============================================================================
/**
 * FADER STYLE INFO STRUCTURE
 * 
 * Holds all dimension and asset information for a fader style.
 * All dimensions are in SCALED pixels (multiplied by uiScale).
 * Spritesheet dimensions are at 4x resolution (divide by 4 for display size).
 */
struct FaderStyleInfo
{
    // TRACK DIMENSIONS (scaled by uiScale)
    float trackWidth;             // Width of the track SVG in pixels
    float trackHeight;            // Height of the track SVG in pixels
    
    // THUMB DIMENSIONS (scaled by uiScale) - from SVG asset
    float thumbWidth;             // Width of thumb graphic (not currently used for vertical)
    float thumbHeight;            // Height of thumb graphic (affects text positioning)
    
    // VALUE TEXT TRAVEL LIMITS (scaled by uiScale)
    float thumbInset;             // Distance from track edges where value text stops
                                  // 0 = text travels full track height
                                  // Increase to keep text away from track edges
    
    // VALUE LABEL POSITION OFFSET (scaled by uiScale)
    float trackYOffset;           // Offset to nudge value label position ONLY
                                  // (does NOT affect the track/frame SVG position)
                                  // For VERTICAL sliders: Y offset (positive = down, negative = up)
                                  // For HORIZONTAL sliders: X offset (positive = right, negative = left)
                                  // Useful for fine-tuning visual alignment within the track
    
    // SPRITESHEET INFO (at 4x resolution)
    int spritesheetTotalFrames;   // Number of animation frames (= track height for vertical)
    int spritesheetFrameWidth;    // Width of each frame at 4x scale (trackWidth × 4)
    int spritesheetFrameHeight;   // Height of each frame at 4x scale (trackHeight × 4)
    
    // ASSET LOCATION
    juce::String folderName;      // Folder name in assets/ directory
    
    // ORIENTATION
    bool isHorizontal;            // false = vertical slider, true = horizontal slider
    
    // TEXT EDITOR DIMENSIONS (for manual value entry via double-click)
    float textEditorWidth;        // Width of text editor box in pixels
    float textEditorPadding;      // Horizontal padding inside text editor
};

//==============================================================================
/**
 * Reusable slider module with label and value display.
 * Supports multiple fader sizes via FaderStyle enum.
 * 
 * Features:
 * - SVG-based track and thumb rendering (via CustomLookAndFeel)
 * - PNG spritesheet-based animated fill bar
 * - Parameter name label below/beside slider
 * - Value display inside thumb (moves with slider)
 * - Customizable accent color with pre-cached color variants
 * - Dynamic parameter reassignment (useful for context-switching UIs)
 * - Multiple fader sizes and orientations
 * 
 * Usage:
 *   SliderModule mySlider {"GAIN", FaderStyle::Fader_38x170};
 *   mySlider.setAccentColour (juce::Colours::blue);
 *   mySlider.attachToParameter (apvts, "gain");
 *   // Later, reassign to different parameter:
 *   mySlider.attachToParameter (apvts, "gain_tap2");
 */
class SliderModule : public juce::Component
{
public:
    //==========================================================================
    // FADER STYLE LOOKUP
    //==========================================================================
    /** Get dimension and asset info for a given fader style */
    static FaderStyleInfo getStyleInfo (FaderStyle style);
    
    /** Get the assets base path */
    static juce::String getAssetsBasePath()
    {
        return "/Users/alistairkerley/Documents/xCode Developments/AKSurroundDelay/assets";
    }
    
    /** Get ideal width for a given fader style (static convenience method) */
    static int getIdealWidthForStyle (FaderStyle style)
    {
        auto info = getStyleInfo (style);
        if (info.isHorizontal)
            return (int)(info.trackHeight + componentPaddingLeft + componentPaddingRight);
        return (int)(info.trackWidth + componentPaddingLeft + componentPaddingRight);
    }
    
    /** Get ideal height for a given fader style (static convenience method) */
    static int getIdealHeightForStyle (FaderStyle style)
    {
        auto info = getStyleInfo (style);
        if (info.isHorizontal)
            return (int)(componentPaddingTop + info.trackWidth + labelSpacing + labelHeight + componentPaddingBottom);
        return (int)(componentPaddingTop + info.trackHeight + labelSpacing + labelHeight + componentPaddingBottom);
    }
    
    /** Get ideal width for default style (backward compatibility) */
    static int getIdealWidth() { return getIdealWidthForStyle (FaderStyle::Fader_38x170); }
    
    /** Get ideal height for default style (backward compatibility) */
    static int getIdealHeight() { return getIdealHeightForStyle (FaderStyle::Fader_38x170); }
    
    //==========================================================================
    // UI SCALING
    // Change uiScale to resize all faders proportionally (1.0 = 100%, 1.5 = 150%)
    //==========================================================================
    static constexpr float uiScale = 1.8f;  // Master scale factor - affects all dimensions below
    
    //==========================================================================
    // COMPONENT LAYOUT (all values scaled by uiScale)
    // These define the total component bounds around the slider track
    // 
    // Component layout (vertical slider):
    // ┌─────────────────────────────────────────┐
    // │  componentPaddingTop (8px)              │
    // │  ┌─────────────────────────────────┐    │
    // │  │                                 │    │
    // │  │     TRACK (trackHeight px)      │    │
    // │  │                                 │    │
    // │  └─────────────────────────────────┘    │
    // │  labelSpacing (4px)                     │
    // │  LABEL (labelHeight 12px)               │
    // │  componentPaddingBottom (4px)           │
    // └─────────────────────────────────────────┘
    //   ├─paddingLeft─┤ track ├─paddingRight─┤
    //==========================================================================
    static constexpr float componentPaddingTop = 8.0f * uiScale;    // Space above track
    static constexpr float componentPaddingBottom = 4.0f * uiScale; // Space below label
    static constexpr float componentPaddingLeft = 8.0f * uiScale;   // Space left of track
    static constexpr float componentPaddingRight = 8.0f * uiScale;  // Space right of track
    static constexpr float labelSpacing = 4.0f * uiScale;           // Gap between track bottom and label top
    static constexpr float labelHeight = 12.0f * uiScale;           // Height for parameter name label
    
    //==========================================================================
    // TEXT STYLING
    //==========================================================================
    static constexpr float valueFontSize = 9.0f * uiScale;  // Font for value text on slider ("0.50", "100ms")
    static constexpr float labelFontSize = 10.0f * uiScale; // Font for parameter name below ("GAIN", "DELAY")
    static inline const juce::Colour labelTextColour {0xffaaaaaa}; /* #aaaaaa - Gray label text */
    
    //==========================================================================
    // CONSTRUCTOR & DESTRUCTOR
    //==========================================================================
    /** Create a slider module with the given label text (parameter name) and fader style */
    SliderModule (const juce::String& labelText, FaderStyle style = FaderStyle::Fader_38x170);
    ~SliderModule() override;
    
    //==========================================================================
    // PARAMETER BINDING
    //==========================================================================
    /** 
     * Attach this slider to a parameter in the APVTS.
     * Can be called multiple times to reassign to different parameters.
     * Perfect for context-switching UIs (e.g., switching between taps).
     * 
     * @param apvts     The AudioProcessorValueTreeState containing parameters
     * @param parameterID  The ID of the parameter to control
     */
    void attachToParameter (juce::AudioProcessorValueTreeState& apvts, 
                           const juce::String& parameterID);
    
    /** Detach from current parameter (slider becomes inactive) */
    void detachFromParameter();
    
    //==========================================================================
    // CUSTOMIZATION
    //==========================================================================
    /** Set a custom accent color for this slider (future: could tint track/thumb) */
    void setAccentColour (juce::Colour colour) { accentColour = colour; }
    
    /** Get the current accent color */
    juce::Colour getAccentColour() const { return accentColour; }
    
    /** Set a custom text color for the value display */
    void setValueTextColour (juce::Colour colour) { valueTextColour = colour; }
    
    /** Get the current value text color */
    juce::Colour getValueTextColour() const { return valueTextColour; }
    
    /** Update the label text (useful when reassigning to different parameter) */
    void setLabelText (const juce::String& text);
    
    /** Set the unit suffix displayed after the value (e.g., "dB", "ms", "%", "Hz") */
    void setValueSuffix (const juce::String& suffix) { valueSuffix = suffix; }
    
    /** Get the current value suffix */
    juce::String getValueSuffix() const { return valueSuffix; }
    
    /** Set the number of decimal places for value display */
    void setDecimalPlaces (int places) { valueDecimalPlaces = places; }
    
    /** Get the number of decimal places for value display */
    int getDecimalPlaces() const { return valueDecimalPlaces; }
    
    /** Enable/disable debug border to visualize component bounds */
    void setShowDebugBorder (bool show) { showDebugBorder = show; repaint(); }
    
    /** 
     * Set the interval between discrete steps (0 = continuous).
     * For discrete sliders, set this to the step size.
     * Example: For a 0-9 slider, use setInterval(1.0) to snap to integers.
     */
    void setInterval (double interval) { slider.setRange (slider.getMinimum(), slider.getMaximum(), interval); }
    
    /**
     * Enable pan display mode for L/R horizontal sliders.
     * When enabled, displays "L99" to "R99" format with direction label above value.
     * Also inverts the slider direction so left = negative, right = positive.
     */
    void setUsePanDisplay (bool usePan);
    
    /** Check if pan display mode is enabled */
    bool getUsePanDisplay() const { return usePanDisplay; }
    
    //==========================================================================
    // ACCESSORS
    //==========================================================================
    /** Access the underlying JUCE slider for advanced customization */
    juce::Slider& getSlider() { return slider; }
    
    /** Get the parameter name this slider is displaying */
    juce::String getParameterName() const { return parameterName; }
    
    /** Check if the text editor is currently active */
    bool isTextEditorActive() const { return isEditingValue; }
    
    /** Dismiss the text editor if active (for external use by custom slider) */
    void dismissTextEditor (bool commitValue) { hideTextEditor (commitValue); }
    
    /** Get the current parameter ID this slider is attached to */
    juce::String getCurrentParameterID() const { return currentParameterID; }
    
    /** Get the fader style for this slider */
    FaderStyle getFaderStyle() const { return faderStyle; }
    
    /** Get the style info for this slider */
    const FaderStyleInfo& getStyleInfo() const { return styleInfo; }
    
    /** Get whether this slider is horizontal */
    bool isHorizontal() const { return styleInfo.isHorizontal; }
    
    /** Get the preferred width for this specific slider instance */
    int getPreferredWidth() const
    { 
        if (styleInfo.isHorizontal)
            return (int)(styleInfo.trackHeight + componentPaddingLeft + componentPaddingRight);  // For horizontal, height becomes width
        return (int)(styleInfo.trackWidth + componentPaddingLeft + componentPaddingRight); 
    }
    
    /** Get the preferred height for this specific slider instance */
    int getPreferredHeight() const
    { 
        if (styleInfo.isHorizontal)
            return (int)(componentPaddingTop + styleInfo.trackWidth + labelSpacing + labelHeight + componentPaddingBottom);  // For horizontal, width becomes height
        return (int)(componentPaddingTop + styleInfo.trackHeight + labelSpacing + labelHeight + componentPaddingBottom); 
    }
    
    //==========================================================================
    // COMPONENT OVERRIDES
    //==========================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDoubleClick (const juce::MouseEvent& event) override;
    void mouseDown (const juce::MouseEvent& event) override;
    
private:
    //==========================================================================
    // TEXT ENTRY FOR MANUAL VALUE INPUT
    //==========================================================================
    void showTextEditor();
    void hideTextEditor (bool commitValue);
    void textEditorReturnKeyPressed();
    void textEditorEscapeKeyPressed();
    void textEditorFocusLost();
    
    //==========================================================================
    // MEMBER VARIABLES
    //==========================================================================
    FaderStyle faderStyle;              // The fader style for this instance
    FaderStyleInfo styleInfo;           // Cached style info (dimensions, asset paths)
    
    SliderModuleSlider slider;          // Custom slider that forwards double-clicks to us
    juce::Label nameLabel;              // Parameter name label (below slider)
    
    juce::String parameterName;         // Display name (e.g., "GAIN")
    juce::String currentParameterID;    // Current APVTS parameter ID (e.g., "gain_tap1")
    juce::String valueSuffix;           // Unit suffix for value display (e.g., "dB", "ms", "%")
    int valueDecimalPlaces = 1;         // Decimal precision for value display (default: 2)
    juce::Colour accentColour {0xffffffff};  /* #ffffff */ // Custom accent color (white default)
    juce::Colour valueTextColour {0xffcccccc};  /* #cccccc */ // Custom text color (light grey default)
    bool showDebugBorder = false;       // Show debug border around component bounds
    bool usePanDisplay = false;         // Use L/R pan display mode for horizontal sliders
    
    // Text editor for manual value entry (double-click to activate)
    std::unique_ptr<juce::TextEditor> valueTextEditor;
    bool isEditingValue = false;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    
    // Fill bar images per style (loaded once, shared by instances of same style)
    // Key = FaderStyle enum value
    static std::map<FaderStyle, juce::Image> fillBarImages;
    static std::map<FaderStyle, bool> fillBarImagesLoaded;
    
    // Pre-generated color variants per style
    // Key = {FaderStyle, colorIndex}
    static std::map<std::pair<FaderStyle, int>, juce::Image> colorVariants;
    static std::map<FaderStyle, bool> colorVariantsLoaded;
    
    //==========================================================================
    // HELPER METHODS
    //==========================================================================
    void loadFillBarForStyle();  // Load fill bar spritesheet for this slider's style
    void loadColorVariantsForStyle();  // Load all pre-cached color variants for this style
    void generateColorVariantCacheForStyle();  // Generate and save color variants for this style
    const juce::Image& getVariantForColor (const juce::Colour& colour) const;  // Get nearest variant for this style
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderModule)
};
