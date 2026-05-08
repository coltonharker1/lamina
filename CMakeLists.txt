# CMake configuration for Grains VST Plugin
cmake_minimum_required(VERSION 3.22)

# Project configuration
project(GrainsVST VERSION 0.1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../JUCE ${CMAKE_CURRENT_BINARY_DIR}/JUCE)

# Add sonorous-kit. Prefer a sibling clone for active development across all
# Project Sonorous repos; fall back to fetching from GitHub for casual / first-
# time clones so `cmake -B build` works out of the box.
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../sonorous-kit/CMakeLists.txt)
    message(STATUS "Lamina: using sibling sonorous-kit")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../sonorous-kit
                     ${CMAKE_CURRENT_BINARY_DIR}/sonorous-kit)
else()
    message(STATUS "Lamina: fetching sonorous-kit from GitHub")
    include(FetchContent)
    FetchContent_Declare(
        sonorous_kit
        GIT_REPOSITORY https://github.com/coltonharker1/sonorous-kit.git
        GIT_TAG        v0.2.0)
    FetchContent_MakeAvailable(sonorous_kit)
endif()

# Plugin configuration
juce_add_plugin(GrainsVST
    COMPANY_NAME "Project Sonorous"
    BUNDLE_ID "com.projectsonorous.lamina"
    IS_SYNTH TRUE                                 # MIDI instrument/synth
    NEEDS_MIDI_INPUT TRUE                         # Accepts MIDI notes
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE                  # Automatically install to system
    PLUGIN_MANUFACTURER_CODE PjSo                 # Project Sonorous 4-char code
    PLUGIN_CODE Lmna                              # Lamina 4-char identifier
    FORMATS VST3 AU                               # Build VST3 and AU (macOS)
    PRODUCT_NAME "Lamina")                        # Display name in DAW

# Fonts are now provided by sonorous_visual — no per-plugin BinaryData target
# needed. The Source/Fonts/ TTFs remain in the repo for now in case any
# instrument-specific code still references them; they can be removed once the
# editor is fully migrated.

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
        Source/ModulatedSlider.h
        Source/LookupTables.h)

# Link JUCE modules and the Sonorous visual kit
target_link_libraries(GrainsVST
    PRIVATE
        sonorous_visual
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
