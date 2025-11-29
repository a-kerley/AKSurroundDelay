cmake_minimum_required(VERSION 3.15)

project(SurroundDelay VERSION 1.0.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Point this to your JUCE installation directory
# Using the local JUCE clone in this workspace
if(NOT DEFINED JUCE_DIR)
    set(JUCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/JUCE" CACHE PATH "Path to JUCE framework")
endif()

# Add JUCE
add_subdirectory(${JUCE_DIR} JUCE)

# Plugin configuration
juce_add_plugin(SurroundDelay
    COMPANY_NAME "YourCompany"
    PLUGIN_MANUFACTURER_CODE Ycom
    PLUGIN_CODE Srdl
    FORMATS AU
    PRODUCT_NAME "Surround Delay"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    AU_MAIN_TYPE kAudioUnitType_Effect
)

# Source files
target_sources(SurroundDelay
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

# Link JUCE modules
target_link_libraries(SurroundDelay
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_plugin_client
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
    COMMAND ${CMAKE_COMMAND} -E echo "${CMAKE_BINARY_DIR}/SurroundDelay_artefacts/AU/SurroundDelay.component"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "To install manually:"
    COMMAND ${CMAKE_COMMAND} -E echo "  cp -r ${CMAKE_BINARY_DIR}/SurroundDelay_artefacts/AU/SurroundDelay.component ~/Library/Audio/Plug-Ins/Components/"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Or use the install_au target:"
    COMMAND ${CMAKE_COMMAND} -E echo "  cmake --build . --target install_au"
    COMMAND ${CMAKE_COMMAND} -E echo "==================================================================="
    COMMAND ${CMAKE_COMMAND} -E echo ""
    DEPENDS SurroundDelay
)

# Custom install target for AU
add_custom_target(install_au
    COMMAND ${CMAKE_COMMAND} -E echo "Installing AU to ~/Library/Audio/Plug-Ins/Components/..."
    COMMAND ${CMAKE_COMMAND} -E make_directory "$ENV{HOME}/Library/Audio/Plug-Ins/Components"
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
        "${CMAKE_BINARY_DIR}/SurroundDelay_artefacts/AU/SurroundDelay.component"
        "$ENV{HOME}/Library/Audio/Plug-Ins/Components/SurroundDelay.component"
    COMMAND ${CMAKE_COMMAND} -E echo "Installation complete!"
    DEPENDS SurroundDelay
)
