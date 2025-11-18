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
    COMPANY_NAME "Project Sonaris"
    BUNDLE_ID "com.projectsonaris.somnia"
    IS_SYNTH TRUE                                 # MIDI instrument/synth
    NEEDS_MIDI_INPUT TRUE                         # Accepts MIDI notes
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE                  # Automatically install to system
    PLUGIN_MANUFACTURER_CODE PjSn                 # Project Sonaris 4-char code
    PLUGIN_CODE Somn                              # Somnia 4-char identifier
    FORMATS VST3 AU                               # Build VST3 and AU (macOS)
    PRODUCT_NAME "Somnia")                        # Display name in DAW

# Source files
target_sources(GrainsVST
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/GrainVoice.h
        Source/GrainEngine.h
        Source/LFO.h
        Source/Envelope.h
        Source/Filter.h
        Source/UnifiedVisualization.h
        Source/InteractiveADSREnvelope.h
        Source/ModulatedSlider.h)

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
        juce::juce_dsp
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

# Performance optimizations (safe flags that don't break JUCE)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(GrainsVST PRIVATE
        -O3                      # Maximum optimization
        -funroll-loops          # Unroll loops for better performance
        -fvectorize             # Enable auto-vectorization
        -march=native           # Optimize for the current CPU
        -fno-math-errno         # Don't set errno for math functions (faster)
    )
elseif(MSVC)
    target_compile_options(GrainsVST PRIVATE
        /O2                      # Maximum optimization
        /arch:AVX2              # Use AVX2 instructions if available
    )
endif()
