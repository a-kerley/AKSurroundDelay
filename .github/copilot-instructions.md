# GitHub Copilot Instructions for SurroundDelay Project

## Project Overview
- **Plugin Name**: SurroundDelay
- **Type**: Audio Unit (AU) plugin for macOS
- **Framework**: JUCE (cloned locally in `./JUCE/`)
- **Build System**: CMake 3.15+
- **Language**: C++17
- **IDE**: VS Code with CMake Tools extension
- **Current Status**: Stereo pass-through plugin (surround support to be added)

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
│   ├── PluginProcessor.h       # SurroundDelayAudioProcessor
│   ├── PluginProcessor.cpp
│   ├── PluginEditor.h          # SurroundDelayAudioProcessorEditor
│   └── PluginEditor.cpp
├── build/                      # CMake build directory
└── .vscode/
    └── settings.json           # CMake configuration
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

### Reset AU Cache (only needed first time or after code signing changes)
```bash
killall -9 AudioComponentRegistrar
```

### Testing in Logic Pro
1. Open Logic Pro
2. Navigate: **Audio Units → ClipPoint → Surround Delay**
3. Verify audio pass-through and GUI display

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

### Class Names
- Processor: `SurroundDelayAudioProcessor`
- Editor: `SurroundDelayAudioProcessorEditor`

### Current Channel Configuration
- **Input**: Stereo (2 channels)
- **Output**: Stereo (2 channels)
- See `isBusesLayoutSupported()` for validation logic

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
- **DO NOT auto-build** after making code changes - wait for explicit build request
- When building, **ALWAYS use parallel builds**: `cmake --build . -- -j8`
- When validating the plugin, **ALWAYS** use `auval -v aufx Srdl Ycom`
- Never suggest `auval -a` or scanning all plugins
- Use modern JUCE includes (module-specific, not JuceHeader.h)
- Build artifacts are in `build/SurroundDelay_artefacts/Debug/` (Debug build)
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
