# CMake configuration for Grains VST Plugin
cmake_minimum_required(VERSION 3.22)

# Project configuration
project(GrainsVST VERSION 0.1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE
add_subdirectory(/Users/colton/Downloads/JUCE ${CMAKE_CURRENT_BINARY_DIR}/JUCE)

# Plugin configuration
juce_add_plugin(GrainsVST
    COMPANY_NAME "YourCompany"                    # Change this later
    IS_SYNTH FALSE                                # It's an effect, not a synth
    NEEDS_MIDI_INPUT FALSE                        # No MIDI for MVP
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE                  # Automatically install to system
    PLUGIN_MANUFACTURER_CODE Manu                 # Change to your 4-char code
    PLUGIN_CODE Grns                              # 4-char plugin identifier
    FORMATS VST3 AU                               # Build VST3 and AU (macOS)
    PRODUCT_NAME "Grains")                        # Display name in DAW

# Source files
target_sources(GrainsVST
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/GrainVoice.h
        Source/GrainEngine.h)

# Link JUCE modules
target_link_libraries(GrainsVST
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Compiler definitions
target_compile_definitions(GrainsVST
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)
