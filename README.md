# SurroundDelay - JUCE Audio Unit Plugin

A stereo delay audio plugin built with JUCE, designed for macOS Audio Units (AU) format.

## Project Structure

```
AKSurroundDelay/
├── CMakeLists.txt          # CMake build configuration
├── Source/
│   ├── PluginProcessor.h   # Audio processor header
│   ├── PluginProcessor.cpp # Audio processor implementation
│   ├── PluginEditor.h      # GUI editor header
│   └── PluginEditor.cpp    # GUI editor implementation
└── README.md               # This file
```

## Prerequisites

1. **JUCE Framework**
   - Download from: https://juce.com/get-juce
   - Extract to a known location (e.g., `~/JUCE`)

2. **CMake** (version 3.15 or higher)
   ```bash
   brew install cmake
   ```

3. **Xcode Command Line Tools**
   ```bash
   xcode-select --install
   ```

4. **VS Code Extensions** (recommended)
   - CMake Tools (by Microsoft)
   - C/C++ (by Microsoft)
   - CMake (by twxs)

## Building the Plugin

### Method 1: Using VS Code with CMake Tools Extension

1. **Open the project in VS Code**
   ```bash
   cd "/Users/alistairkerley/Documents/xCode Developments/AKSurroundDelay"
   code .
   ```

2. **Configure CMake**
   - Press `Cmd+Shift+P` to open the Command Palette
   - Type "CMake: Configure" and press Enter
   - If prompted for the JUCE_DIR, either:
     - Set it in the CMake configuration: `-DJUCE_DIR=/path/to/JUCE`
     - Or set an environment variable in your shell profile

3. **Select a Kit**
   - CMake Tools will ask you to select a kit
   - Choose "Clang" or the Xcode toolchain

4. **Build the Plugin**
   - Press `Cmd+Shift+P`
   - Type "CMake: Build" and press Enter
   - Or click the "Build" button in the VS Code status bar
   - Or use the keyboard shortcut: `Shift+F7`

5. **Monitor Build Output**
   - The Output panel will show build progress
   - Look for the installation instructions at the end

### Method 2: Using Terminal

1. **Configure and Build**
   ```bash
   cd "/Users/alistairkerley/Documents/xCode Developments/AKSurroundDelay"
   
   # Create build directory
   mkdir -p build
   cd build
   
   # Configure (replace /path/to/JUCE with your JUCE installation)
   cmake -DJUCE_DIR=/path/to/JUCE ..
   
   # Or if JUCE is in ~/JUCE:
   cmake -DJUCE_DIR=$HOME/JUCE ..
   
   # Build
   cmake --build .
   ```

2. **The AU component will be built to:**
   ```
   build/SurroundDelay_artefacts/AU/SurroundDelay.component
   ```

## Installing the AU Plugin

### Option 1: Automatic Installation
```bash
cd build
cmake --build . --target install_au
```

### Option 2: Manual Installation
```bash
cp -r build/SurroundDelay_artefacts/AU/SurroundDelay.component \
      ~/Library/Audio/Plug-Ins/Components/
```

### Option 3: Auto-copy After Build
The CMakeLists.txt has `COPY_PLUGIN_AFTER_BUILD TRUE`, which automatically copies the plugin to the Components folder after each successful build (when using JUCE 6.1+).

## Testing in Logic Pro

1. **Reset Audio Units Cache** (first time only)
   ```bash
   killall -9 AudioComponentRegistrar
   ```

2. **Launch Logic Pro**
   - Create a new project or open an existing one
   - Add a new Software Instrument or Audio track

3. **Add the Plugin**
   - Click on an insert slot in the channel strip
   - Navigate to: Audio Units → YourCompany → Surround Delay
   - The plugin should appear with a dark gradient background and "Surround Delay" title

4. **Verify Audio Pass-Through**
   - Play audio through the track
   - Audio should pass through cleanly (no processing yet)
   - Check that there are no glitches or dropouts

## Troubleshooting

### Plugin Doesn't Appear in Logic Pro
```bash
# Reset the Audio Unit cache
killall -9 AudioComponentRegistrar

# Validate the plugin
auval -v aufx Srdl Ycom

# Check if the component exists
ls -la ~/Library/Audio/Plug-Ins/Components/SurroundDelay.component
```

### CMake Can't Find JUCE
Set the JUCE_DIR either:
- As a CMake variable: `cmake -DJUCE_DIR=/path/to/JUCE ..`
- As an environment variable in `~/.zshrc`:
  ```bash
  export JUCE_DIR="$HOME/JUCE"
  ```

### Build Errors
1. Ensure Xcode Command Line Tools are installed:
   ```bash
   xcode-select --install
   ```

2. Verify CMake version:
   ```bash
   cmake --version  # Should be 3.15 or higher
   ```

3. Clean and rebuild:
   ```bash
   rm -rf build
   mkdir build
   cd build
   cmake -DJUCE_DIR=$HOME/JUCE ..
   cmake --build .
   ```

## VS Code CMake Tools Quick Reference

| Action | Keyboard Shortcut | Command Palette |
|--------|------------------|-----------------|
| Configure | - | CMake: Configure |
| Build | `Shift+F7` | CMake: Build |
| Clean | - | CMake: Clean |
| Clean Rebuild | - | CMake: Clean Rebuild |
| Select Kit | - | CMake: Select a Kit |
| Select Variant | - | CMake: Select Variant |

## Project Configuration

### Current Features
- ✅ Stereo input/output (2 channels)
- ✅ Audio pass-through (no processing)
- ✅ Minimal GUI with title label
- ✅ AU format only
- ✅ macOS compatible

### Upcoming Features
- ⏳ Surround sound support (5.1, 7.1)
- ⏳ Delay processing (tap delays)
- ⏳ Parameter controls (delay time, feedback, mix)
- ⏳ Enhanced GUI

## Customization

### Changing Plugin Details
Edit `CMakeLists.txt`:
- `COMPANY_NAME`: Your company/developer name
- `PLUGIN_MANUFACTURER_CODE`: 4-character manufacturer ID
- `PLUGIN_CODE`: 4-character plugin ID
- `PRODUCT_NAME`: Display name in DAW

### Adding Parameters
Parameters will be added to `PluginProcessor.h/cpp` using JUCE's `AudioProcessorValueTreeState`.

## Development Notes

- **C++ Standard**: C++17
- **JUCE Version**: Compatible with JUCE 6.x and 7.x
- **Build System**: CMake 3.15+
- **Target Platform**: macOS (AU format)
- **Channel Configuration**: Currently stereo only (2in/2out)

## License

[Add your license here]

## Contact

[Add your contact information here]
