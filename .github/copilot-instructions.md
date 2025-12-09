# GitHub Copilot Instructions for SurroundDelay Project

## Project Overview
- **Plugin Name**: SurroundDelay
- **Type**: Audio Unit (AU) plugin for macOS
- **Framework**: JUCE (cloned locally in `./JUCE/`)
- **Build System**: CMake 3.15+
- **Language**: C++17
- **IDE**: VS Code with CMake Tools extension
- **Current Status**: Delay plugin with multi-channel support (mono, stereo, 5.1, 7.1, multi-mono)

## Plugin Identifiers
- **Manufacturer Code**: `Ycom` (ClipPoint)
- **Plugin Code**: `Srdl` (Surround Delay)
- **Plugin Type**: `aufx` (Audio Unit Effect)
- **Product Name**: "Surround Delay"

## Project Structure
```
AKSurroundDelay/
├── CMakeLists.txt              # JUCE build configuration
├── JUCE/                       # Local JUCE framework (git-ignored)
├── Source/
│   ├── PluginProcessor.h       # TapMatrixAudioProcessor (formerly SurroundDelay)
│   ├── PluginProcessor.cpp
│   ├── PluginEditor.h          # TapMatrixAudioProcessorEditor
│   ├── PluginEditor.cpp
│   ├── CustomLookAndFeel.h     # Custom LookAndFeel for SVG-based UI
│   ├── CustomLookAndFeel.cpp
│   ├── SliderModule.h          # Reusable slider component with FaderStyle support
│   └── SliderModule.cpp
├── assets/                     # Fader assets organized by size
│   ├── Fader 38 x 170/         # Standard vertical fader (default)
│   │   ├── Fader 38 x 170_frame.svg
│   │   ├── Fader 38 x 170_thumb.svg
│   │   ├── Fader 38 x 170_sprite_sheet.png
│   │   └── Fader 38 x 170_color0-9.png  # Pre-generated color variants
│   ├── Fader 32 x 129/         # Medium vertical fader
│   ├── Fader 32 x 129 Front-Back/  # Medium fader with Front/Back labels
│   ├── Fader 28 x 84 Horizontal Left-Right/  # Horizontal fader
│   ├── Fader 22 x 170/         # Slim vertical fader
│   └── Fader 22 x 79/          # Small vertical fader
├── build/                      # CMake build directory
└── .vscode/
    └── settings.json           # CMake configuration
```

## UI Component System

### SliderModule - Reusable Slider Component
A fully-featured slider component supporting multiple fader styles:
- **Multiple Sizes**: 6 fader styles via `FaderStyle` enum
- **SVG Rendering**: Track and thumb loaded from style-specific folder
- **PNG Spritesheet**: Animated fill bar with pre-cached color variants
- **Value Display**: Shows parameter value inside the moving thumb
- **Parameter Label**: Displays parameter name below/beside slider
- **Color Tinting**: Per-slider accent color with instant color lookup
- **Dynamic Reassignment**: Can be reassigned to different parameters on-the-fly
- **Horizontal Support**: `Fader_28x84_HorizontalLeftRight` for horizontal sliders

#### Key Features
```cpp
// Create a slider module (default style: Fader_38x170)
SliderModule gainSlider {"GAIN"};

// Create with specific fader style
SliderModule smallSlider {"VOL", FaderStyle::Fader_22x79};
SliderModule panSlider {"PAN", FaderStyle::Fader_28x84_HorizontalLeftRight};

// Set custom color (tints SVG track and thumb)
gainSlider.setAccentColour (juce::Colours::blue);

// Attach to parameter
gainSlider.attachToParameter (apvts, "gain");

// Later, reassign to different parameter (useful for context-switching UIs)
gainSlider.attachToParameter (apvts, "gain_tap2");
gainSlider.setLabelText ("GAIN TAP 2");

// Detach from parameter
gainSlider.detachFromParameter();
```

#### How Color Tinting Works
1. Each `SliderModule` stores an `accentColour` (default: white)
2. When drawing, `CustomLookAndFeel::drawLinearSlider()` gets the parent `SliderModule`
3. It clones the SVG drawables and uses `replaceColour()` to tint white/black/stroke colors
4. Both track and thumb SVGs are recolored with the accent color
5. This allows per-slider colors without modifying the original SVG assets

Example: Hue control slider updates colors dynamically:
```cpp
// In editor's onValueChange callback
float hue = hueSlider.getSlider().getValue() / 360.0f;
auto colour = juce::Colour::fromHSV (hue, 0.8f, 0.9f, 1.0f);
mixSlider.setAccentColour (colour);
hueSlider.setAccentColour (colour);
mixSlider.repaint();
hueSlider.repaint();
```

#### Dynamic UI Scaling
SliderModules now support dynamic scaling from 1.0x to 3.0x:
```cpp
// Scale factor is set from parent editor
slider.setScaleFactor (1.5f);  // 150% scale

// Get current scale factor
float scale = slider.getScaleFactor();

// Preferred dimensions account for current scale
int width = slider.getPreferredWidth();
int height = slider.getPreferredHeight();
```

**Base constants** (at 1.0x scale) are defined as `base*` static members:
- `baseComponentPaddingTop`, `baseComponentPaddingBottom`, etc.
- `baseValueFontSize`, `baseLabelFontSize`

**Scaled accessors** use current scale factor automatically:
- `componentPaddingTop()`, `labelSpacing()`, `labelHeight()`
- `valueFontSize()`, `labelFontSize()`

#### Customizable Constants (SliderModule.h)
```cpp
// Dimensions (match SVG assets)
static constexpr float trackWidth = 38.0f;
static constexpr float trackHeight = 170.0f;
static constexpr float thumbWidth = 34.0f;
static constexpr float thumbHeight = 13.0f;
static constexpr float thumbPadding = 2.0f;

// Text styling
static constexpr float valueFontSize = 9.0f;
static constexpr float labelFontSize = 10.0f;
static constexpr int valueDecimalPlaces = 2;
static inline const juce::Colour valueTextColour {0xff1a1a1a};
static inline const juce::Colour labelTextColour {0xffaaaaaa};
```

### CustomLookAndFeel
- Loads SVG assets from absolute path: `/Users/alistairkerley/Documents/xCode Developments/AKSurroundDelay/assets/`
- Handles SVG color replacement for per-slider tinting
- Replaces white (#FFFFFF), black (#000000), and stroke color (#F2F2F7) with accent color
- Smart value formatting: 0-1 range shows decimals, larger values show integers
- Constrains thumb travel with `jlimit()` to prevent overflow (2px padding)

### UI Scaling System
The plugin window supports dynamic resizing from 1.0x to 3.0x scale with locked aspect ratio.

#### Window Specifications
- **Base size (1x)**: 1100×820 pixels
- **Aspect ratio**: 55:41 (locked)
- **Scale range**: 1.0x to 3.0x
- **Scale step**: 0.1 (snapped to nearest tenth)
- **Max size (3x)**: 3300×2460 pixels

#### Key Files
- **ResizeHandle.h**: Drag handle component for bottom-right corner
- **UIScaling namespace**: Constants and helpers in `ResizeHandle.h`

#### How It Works
1. User drags the resize handle in bottom-right corner
2. `ResizeHandle::onResize` callback fires with new scale factor
3. `TapMatrixAudioProcessorEditor::setUIScaleFactorAndResize()` is called
4. Editor saves scale to processor state (`audioProcessor.setUIScaleFactor()`)
5. Editor calls `updateAllComponentScales()` to update all child components
6. Editor resizes window via `setSize()`
7. Scale factor persists with plugin state (getStateInformation/setStateInformation)

#### Getting Scale Factor in Components
```cpp
// From within a child component
if (auto* editor = dynamic_cast<TapMatrixAudioProcessorEditor*>(getParentComponent()))
{
    float scale = editor->getScaleFactor();
}

// SliderModule has built-in scale support
slider.setScaleFactor (scale);  // Updates internal styleInfo with scaled dimensions
float currentScale = slider.getScaleFactor();
```

#### Adding New Scalable Components
When creating new UI components that need to scale:
1. Add a `float currentScaleFactor = 1.0f;` member variable
2. Add a `void setScaleFactor(float scale)` method that:
   - Updates `currentScaleFactor`
   - Recalculates any cached dimensions
   - Calls `resized()` and `repaint()`
3. Use `currentScaleFactor` to multiply all dimension constants
4. Register the component in `updateAllComponentScales()` in the editor

## UI Style Guide

### Design Philosophy
- **Dark, minimal aesthetic** - clean and unobtrusive
- **Consistent greyscale palette** - color used sparingly for accents and state
- **Subtle depth** - semi-transparent layers, no harsh shadows
- **Smooth animations** - ease-out transitions for all state changes

### Color Palette

#### Background Colors
```cpp
static inline const juce::Colour backgroundDark {0xff1a1a1a};      /* #1a1a1a - Main plugin background */
static inline const juce::Colour backgroundMid {0xff2a2a2a};       /* #2a2a2a - Control backgrounds */
static inline const juce::Colour backgroundLight {0xff3a3a3a};     /* #3a3a3a - Dimmed/inactive elements */
```

#### Border & Stroke Colors
```cpp
static inline const juce::Colour borderColour {0xff4a4a4a};        /* #4a4a4a - Control borders */
static inline const juce::Colour borderSubtle {0xff3a3a3a};        /* #3a3a3a - Subtle separators */
```

#### Interactive Element Colors
```cpp
static inline const juce::Colour pillColour {0xff5a5a5a};          /* #5a5a5a - Selected/active pill */
static inline const juce::Colour pillDimmedColour {0xff3a3a3a};    /* #3a3a3a - Custom/inactive state */
```

#### Text Colors
```cpp
static inline const juce::Colour textPrimary {0xffffffff};         /* #ffffff - Primary text, active labels */
static inline const juce::Colour textSecondary {0xffaaaaaa};       /* #aaaaaa - Secondary text, labels */
static inline const juce::Colour textDimmed {0xff888888};          /* #888888 - Inactive/dimmed text */
static inline const juce::Colour textOnAccent {0xff1a1a1a};        /* #1a1a1a - Text on bright backgrounds */
```

#### 3D Viewport Colors
```cpp
static inline const juce::Colour viewport3DBackground {0xff1a1a1a}; /* #1a1a1a - 3D scene background */
static inline const juce::Colour roomWallsColour {0xff151515};      /* #151515 @ 60% alpha - Semi-transparent walls */
static inline const juce::Colour roomEdgesColour {0xffffffff};      /* #ffffff - Wireframe edges */
static inline const juce::Colour gridColour {0xff666666};           /* #666666 - Floor grid */
static inline const juce::Colour sphereColour {0xff808080};         /* #808080 - Listener/tap markers */
```

### Typography

#### Font Sizes
```cpp
static constexpr float fontSizeSmall = 9.0f;    // Value displays, fine detail
static constexpr float fontSizeMedium = 10.0f;  // Labels, secondary text
static constexpr float fontSizeLarge = 12.0f;   // Headings, primary controls
```

#### Font Weights
- **Bold** for control labels (ANGLE, LEFT, TOP, etc.)
- **Regular** for values and secondary information

### Corner Radii & Borders

#### Standard Values
```cpp
static constexpr float cornerRadiusSmall = 3.0f;   // Small buttons, tags
static constexpr float cornerRadiusMedium = 5.0f;  // Segmented controls, panels
static constexpr float cornerRadiusLarge = 8.0f;   // Large containers, modals

static constexpr float borderWidthThin = 1.0f;     // Standard control borders
static constexpr float borderWidthMedium = 1.5f;   // Emphasis borders
```

### Segmented Control Style (ViewPresetSelector)
```cpp
static constexpr float cornerRadius = 5.0f;        // Outer container
static constexpr float borderWidth = 1.0f;         // Container border
static constexpr float pillPadding = 2.0f;         // Padding between pill and container edge
static constexpr float animationSpeed = 0.15f;     // 0-1 per frame, ease-out

// States
// - Active: pillColour (#5a5a5a), textPrimary (#ffffff)
// - Dimmed (custom): pillDimmedColour (#3a3a3a), textDimmed (#888888)
```

### Animation Guidelines

#### Timing
```cpp
static constexpr float animationSpeedFast = 0.15f;   // Quick transitions (pill slide)
static constexpr float animationSpeedMedium = 0.08f; // Standard transitions (view rotation)
static constexpr float animationSpeedSlow = 0.05f;   // Slow, deliberate transitions
```

#### Easing
- **Ease-out** for most UI animations: `value += (target - value) * speed`
- **Linear** for continuous rotations or loops
- **60 FPS** timer for smooth animations

### State Indication

#### Active/Selected
- Full brightness text (textPrimary)
- Visible pill/highlight background (pillColour)

#### Inactive/Unselected  
- Dimmed text (textDimmed or textSecondary)
- No highlight background

#### Custom/Modified State
- Dimmed pill (pillDimmedColour)
- Dimmed text across all options
- Indicates user has customized beyond presets

### OpenGL 3D Viewport Style

#### Room Visualization
- **Walls**: Semi-transparent filled triangles for depth testing
- **Edges**: White wireframe (1.5px line width)
- **Floor grid**: 4×4 divisions, subtle grey
- **No floor fill** - grid only for visibility

#### Objects (Spheres/Markers)
- Solid filled geometry (triangles, not wireframe)
- Grey tones (0.35-0.5 brightness)
- Proper depth testing against room walls

#### Camera Defaults
```cpp
static constexpr float defaultAzimuth = 225.0f;    // Back-left isometric view
static constexpr float defaultElevation = 30.0f;   // Slight overhead
static constexpr float defaultZoom = 4.5f;         // Comfortable framing
```

## Build Instructions

### VS Code Method (Preferred)
1. Configure: `Cmd+Shift+P` → "CMake: Configure"
2. Build: `Shift+F7` or `Cmd+Shift+P` → "CMake: Build"
3. Plugin auto-installs to `~/Library/Audio/Plug-Ins/Components/`

### Terminal Method
```bash
cd build
cmake --build .
# Plugin is automatically installed during build
```

### Clean Rebuild
```bash
rm -rf build
mkdir build && cd build
cmake ..
cmake --build .
```

## Testing & Validation

### ⚠️ IMPORTANT: Plugin Validation
**ALWAYS use the specific plugin identifier - DO NOT run full AU scan:**
```bash
# ✅ CORRECT - validates only SurroundDelay (fast, ~5 seconds)
auval -v aufx Srdl Ycom

# ❌ WRONG - scans ALL plugins (slow, many minutes)
auval -a
```

### Reset AU Cache (when plugin metadata/channel configs change)
```bash
# 1. Reset system AU cache
killall -9 AudioComponentRegistrar

# 2. Force Logic Pro to rescan (REQUIRED - cache deletion alone doesn't work)
# Open Logic Pro → Preferences → Plug-in Manager → Select "Surround Delay" → Reset & Rescan Selection
```

**IMPORTANT**: Logic Pro maintains its own internal plugin cache that is **separate** from the system AU cache. Simply deleting cache files or killing AudioComponentRegistrar will NOT cause Logic to rescan. You MUST manually trigger a rescan from Logic's Plugin Manager.

### Testing in Logic Pro
1. Open Logic Pro
2. Navigate: **Audio Units → ClipPoint → Surround Delay**
3. Verify audio pass-through and GUI display
4. Test on multiple track formats (mono, stereo, 5.1, 7.1) to verify channel flexibility

## Code Conventions

### JUCE Header Includes
Modern JUCE doesn't use `JuceHeader.h`. Use specific module includes:

```cpp
// ✅ CORRECT
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

// ❌ WRONG (deprecated)
#include <JuceHeader.h>
```

### Color Definitions
ALWAYS use 8-digit hex format (0xAARRGGBB) with hex color comments for VS Code color picker:

```cpp
// ✅ CORRECT - enables VS Code color picker
juce::Colour myColor {0xff1a1a1a}; /* #1a1a1a */
static inline const juce::Colour accentColor {0xffff5500}; /* #ff5500 */

// ❌ WRONG - no color picker support
juce::Colour myColor {0xff1a1a1a}; // missing hex comment
juce::Colour myColor = juce::Colours::blue; // named colors don't show picker
```

### Class Names
- Processor: `SurroundDelayAudioProcessor`
- Editor: `SurroundDelayAudioProcessorEditor`

### Current Channel Configuration
- **Supported Formats**: Mono, Stereo, Multi-mono (up to 8 channels), 5.1, 7.1
- **Channel Capabilities**: [1,1] [1,2] [1,3] [1,4] [1,5] [1,6] [1,7] [1,8] [2,2] [2,6] [2,8] [3,3] [4,4] [5,5] [6,6] [7,7] [8,8]
- **Upmixing**: Mono → any format, Stereo → 5.1/7.1
- See `isBusesLayoutSupported()` for dynamic channel validation logic

## Development Workflow

### After Code Changes
1. **Review changes** - don't auto-build after every edit
2. **Build when ready**: `Shift+F7` in VS Code (use parallel builds)
3. **Validate**: `auval -v aufx Srdl Ycom` (only if metadata changed)
4. **Test**: Quit and reopen Logic Pro to reload plugin

### Build Configuration
- **Default**: Debug build (faster compilation, larger binary)
- **For testing performance**: Use Release build
- **Parallel builds**: Always use `-j` flag for faster compilation
- **ccache**: Installed and enabled for build caching

### CMake Configuration
- JUCE path set in `.vscode/settings.json`: `${workspaceFolder}/JUCE`
- Build directory: `${workspaceFolder}/build`
- Plugin auto-copies after successful build
- Current build type: **Debug** (see `SurroundDelay_artefacts/Debug/`)

## Known Build Warnings (Safe to Ignore)
- Font constructor deprecation warnings (using FontOptions is optional)
- "has no symbols" warnings for ARA/LV2 modules (not used in AU-only build)

## Future Development Tasks
- [ ] Add delay processing (tap delays)
- [ ] Implement surround sound support (5.1, 7.1)
- [ ] Add parameters (delay time, feedback, mix, etc.)
- [ ] Enhanced GUI with controls
- [ ] Preset management

## Troubleshooting

### Plugin doesn't appear in Logic
```bash
killall -9 AudioComponentRegistrar
auval -v aufx Srdl Ycom
```

### Build fails with missing JUCE
- Verify: `ls JUCE/modules/juce_core/juce_core.h`
- If missing: `git clone https://github.com/juce-framework/JUCE.git`

### Code signing issues
Plugin is ad-hoc signed during build. For distribution, proper code signing will be needed.

## AI Assistant Guidelines

### Standard Workflow After Code Changes
**ALWAYS follow this sequence after making ANY code changes:**

1. **Build with parallel compilation**:
   ```bash
   cd /Users/alistairkerley/Documents/xCode\ Developments/AKSurroundDelay/build && cmake --build . -- -j8
   ```

2. **Run standalone app with debug output**:
   ```bash
   "/Users/alistairkerley/Documents/xCode Developments/AKSurroundDelay/build/TapMatrix_artefacts/Debug/Standalone/TapMatrix.app/Contents/MacOS/TapMatrix" 2>&1
   ```
   - Run as **background process** (`isBackground: true`) so it doesn't block
   - Print the debug console output to terminal in VS Code
   - **NEVER** use `open` command - always use direct executable path with `2>&1` to capture stderr

3. **Only when explicitly requested**: Validate with `auval -v aufx Srdl Ycom`

### General Guidelines
- **DO NOT auto-build** after making code changes - wait for explicit build request
- When building, **ALWAYS use parallel builds**: `cmake --build . -- -j8`
- When validating the plugin, **ALWAYS** use `auval -v aufx Srdl Ycom`
- Never suggest `auval -a` or scanning all plugins
- Use modern JUCE includes (module-specific, not JuceHeader.h)
- Build artifacts are in `build/TapMatrix_artefacts/Debug/` (Debug build)
- Plugin automatically installs to Components folder
- When adding features, maintain stereo compatibility first
- Follow JUCE 7.x best practices (FontOptions, modern API patterns)
- Only suggest `killall -9 AudioComponentRegistrar` when plugin metadata changes

## Quick Reference Commands
```bash
# Build (with parallel compilation)
cd build && cmake --build . -- -j8

# Validate (FAST - specific plugin only)
auval -v aufx Srdl Ycom

# Reset AU cache (only when metadata changes)
killall -9 AudioComponentRegistrar

# Check installation
ls -la ~/Library/Audio/Plug-Ins/Components/Surround\ Delay.component

# Clean build
rm -rf build && mkdir build && cd build && cmake .. && cmake --build . -- -j8
```

## Control Groups & Value Display Modes

### Overview
Control groups bundle related sliders (e.g., position controls for L/R, F/B, Height) into a single reusable component. This section documents patterns for implementing control groups with specialized value formatting.

### ValueDisplayMode Enum
SliderModule supports specialized value formatting via `ValueDisplayMode`:

```cpp
enum class ValueDisplayMode
{
    Standard,      // Default: shows raw value (e.g., "0.50", "100")
    PanLeftRight,  // Bipolar L/R: "L100", "C", "R100"
    FrontBack,     // Bipolar F/B: "F100", "C", "B100"
    Percent        // 0-100%: "0%", "50%", "100%"
};

// Usage
slider.setValueDisplayMode (ValueDisplayMode::PanLeftRight);
```

### Parameter ID Naming Convention
**CRITICAL**: Parameter IDs follow the pattern `paramName + tapIndex` where tapIndex is **1-based**:

```cpp
// ✅ CORRECT - 1-based indexing
juce::String paramId = "panX" + juce::String (tapIndex);  // panX1, panX2, panX3...

// ❌ WRONG - 0-based indexing
juce::String paramId = "panX" + juce::String (tapIndex);  // panX0 doesn't exist!
```

Common parameter patterns:
- Position: `panX1`, `panY1`, `panZ1` (L/R, F/B, Height)
- Delay: `delayTime1`, `feedback1`, `mix1`
- Filter: `filterFreq1`, `filterQ1`

### Color Palette System
ColorPalette provides paired colors for each tap - an accent color AND a readable text color:

```cpp
// ColorPalette.h structure
struct ColourPair
{
    juce::Colour background;  // Accent color (for track tinting)
    juce::Colour text;        // Text color readable on that background
};

static inline const std::array<ColourPair, 8> palettePairs = {{
    { {0xffE0B0FF}, {0xff2a1a3a} },  // Mauve / Dark purple text
    { {0xffFFB347}, {0xff3a2a1a} },  // Pastel orange / Dark orange text
    // ... etc
}};
```

**Usage in control groups:**
```cpp
void setColour (juce::Colour accent, int colourIndex)
{
    // Accent color for track/thumb tinting
    lrSlider.setAccentColour (accent);
    
    // Associated text color for value display (readable on thumb)
    auto textColour = ColorPalette::palettePairs[colourIndex].text;
    lrSlider.setValueTextColour (textColour);
}
```

### Center Detection for Bipolar Parameters
For parameters with -1 to +1 range, detect center position with tolerance:

```cpp
// In CustomLookAndFeel::drawLinearSlider()
bool isAtCenter = std::abs (currentValue) < 0.005f;  // 0.5% tolerance

if (isAtCenter)
    valueText = "C";
else if (currentValue < 0)
    valueText = "L" + juce::String (static_cast<int> (std::abs (currentValue) * 100));
else
    valueText = "R" + juce::String (static_cast<int> (currentValue * 100));
```

### Text Editor Value Formatting
When user double-clicks to edit a value, small-range parameters need decimal places:

```cpp
// In SliderModule's TextEditor creation
float range = slider.getMaximum() - slider.getMinimum();
int decimalPlaces = (range <= 2.0f) ? 2 : 0;  // Bipolar (-1 to 1) needs decimals

editor.setText (juce::String (slider.getValue(), decimalPlaces), false);
```

### Inactive State Handling
When a slider is disabled, grey out both the SVG track and the value text:

```cpp
// Value text color
juce::Colour SliderModule::getValueTextColour() const
{
    if (! slider.isEnabled())
        return juce::Colour {0xff666666};  // Grey when inactive
    return valueTextColour;
}

// Track SVG tinting (in CustomLookAndFeel)
auto tintColour = slider.isEnabled() ? accentColour : juce::Colour {0xff666666};
```

### Control Group Implementation Pattern
Example structure for a position control group:

```cpp
class PositionControlGroup : public juce::Component
{
public:
    PositionControlGroup (int tapIndex);
    
    void attachToParameters (juce::AudioProcessorValueTreeState& apvts);
    void setColour (juce::Colour accent, int colourIndex);
    void setValueTextColour (juce::Colour colour);
    void setScaleFactor (float scale);
    
private:
    int tapIndex;
    SliderModule lrSlider;  // Horizontal, PanLeftRight mode
    SliderModule fbSlider;  // Vertical, FrontBack mode
    SliderModule heightSlider;  // Small vertical, Percent mode
};

// Constructor - set up display modes
PositionControlGroup::PositionControlGroup (int tapIdx)
    : tapIndex (tapIdx),
      lrSlider ("L/R", FaderStyle::Fader_28x84_HorizontalLeftRight),
      fbSlider ("F/B", FaderStyle::Fader_32x129_FrontBack),
      heightSlider ("HGT", FaderStyle::Fader_22x79)
{
    lrSlider.setValueDisplayMode (ValueDisplayMode::PanLeftRight);
    fbSlider.setValueDisplayMode (ValueDisplayMode::FrontBack);
    heightSlider.setValueDisplayMode (ValueDisplayMode::Percent);
    
    addAndMakeVisible (lrSlider);
    addAndMakeVisible (fbSlider);
    addAndMakeVisible (heightSlider);
}

// Attach with correct parameter IDs (1-based!)
void PositionControlGroup::attachToParameters (juce::AudioProcessorValueTreeState& apvts)
{
    lrSlider.attachToParameter (apvts, "panX" + juce::String (tapIndex));
    fbSlider.attachToParameter (apvts, "panY" + juce::String (tapIndex));
    heightSlider.attachToParameter (apvts, "panZ" + juce::String (tapIndex));
}
```

### Common Pitfalls

1. **Wrong parameter IDs**: Always use 1-based indexing (`panX1` not `panX0`)
2. **Missing text color**: Set both accent AND text color from ColorPalette
3. **Text editor shows "0"**: Check range detection - use 2 decimals for ranges ≤2.0
4. **Slider not responding**: Verify parameter exists in APVTS with exact ID
5. **Wrong center detection**: Use tolerance (0.005f) not exact equality

## Contact & License
- Developer: [Your Name]
- License: [Your License]
- Repository: [Your Repo URL]
