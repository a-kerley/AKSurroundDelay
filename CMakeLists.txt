cmake_minimum_required(VERSION 3.15)

project(TapMatrix VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable ccache if available for faster rebuilds
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    message(STATUS "ccache found and enabled")
endif()

# Point this to your JUCE installation directory
# Using the local JUCE clone in this workspace
if(NOT DEFINED JUCE_DIR)
    set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/JUCE" CACHE PATH "Path to JUCE framework")
endif()

# Add JUCE
add_subdirectory(${JUCE_DIR} JUCE)

# Plugin configuration
juce_add_plugin(TapMatrix
    COMPANY_NAME "ClipPoint"
    PLUGIN_MANUFACTURER_CODE Ycom
    PLUGIN_CODE TpMx
    FORMATS AU Standalone
    PRODUCT_NAME "TapMatrix"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    AU_MAIN_TYPE kAudioUnitType_Effect
)

# Source files
target_sources(TapMatrix
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/CustomLookAndFeel.cpp
        Source/SliderModule.cpp
)

# Link JUCE modules
target_link_libraries(TapMatrix
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_plugin_client
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Build instructions (shown in CMake output)
add_custom_target(install_instructions ALL
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "==================================================================="
    COMMAND ${CMAKE_COMMAND} -E echo "BUILD COMPLETE"
    COMMAND ${CMAKE_COMMAND} -E echo "==================================================================="
    COMMAND ${CMAKE_COMMAND} -E echo "The AU component is located at:"
    COMMAND ${CMAKE_COMMAND} -E echo "${CMAKE_BINARY_DIR}/TapMatrix_artefacts/AU/TapMatrix.component"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "To install manually:"
    COMMAND ${CMAKE_COMMAND} -E echo "  cp -r ${CMAKE_BINARY_DIR}/TapMatrix_artefacts/AU/TapMatrix.component ~/Library/Audio/Plug-Ins/Components/"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Or use the install_au target:"
    COMMAND ${CMAKE_COMMAND} -E echo "  cmake --build . --target install_au"
    COMMAND ${CMAKE_COMMAND} -E echo "==================================================================="
    COMMAND ${CMAKE_COMMAND} -E echo ""
    DEPENDS TapMatrix
)

# Custom install target for AU
add_custom_target(install_au
    COMMAND ${CMAKE_COMMAND} -E echo "Installing AU to ~/Library/Audio/Plug-Ins/Components/..."
    COMMAND ${CMAKE_COMMAND} -E make_directory "$ENV{HOME}/Library/Audio/Plug-Ins/Components"
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
        "${CMAKE_BINARY_DIR}/TapMatrix_artefacts/AU/TapMatrix.component"
        "$ENV{HOME}/Library/Audio/Plug-Ins/Components/TapMatrix.component"
    COMMAND ${CMAKE_COMMAND} -E echo "Installation complete!"
    DEPENDS TapMatrix
)
