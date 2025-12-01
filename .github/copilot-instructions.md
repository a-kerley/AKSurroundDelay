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
│   ├── SliderModule.h          # Reusable vertical slider component
│   └── SliderModule.cpp
├── assets/                     # SVG assets for UI elements
│   ├── SliderTrack.svg         # 38×170px slider track (white)
│   └── SliderThumb.svg         # 34×13px slider thumb (white)
├── build/                      # CMake build directory
└── .vscode/
    └── settings.json           # CMake configuration
```

## UI Component System

### SliderModule - Reusable Vertical Slider
A fully-featured, SVG-based vertical slider component with:
- **SVG Rendering**: Track (38×170px) and thumb (34×13px) loaded from `assets/`
- **Value Display**: Shows parameter value inside the moving thumb
- **Parameter Label**: Displays parameter name below the slider
- **Color Tinting**: Per-slider accent color that tints both track and thumb SVGs
- **Dynamic Reassignment**: Can be reassigned to different parameters on-the-fly
- **Configurable Styling**: All dimensions, fonts, and colors defined as static constants

#### Key Features
```cpp
// Create a slider module
SliderModule gainSlider {"GAIN"};

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

## Contact & License
- Developer: [Your Name]
- License: [Your License]
- Repository: [Your Repo URL]
