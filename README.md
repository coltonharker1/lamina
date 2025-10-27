# Grains VST - Granular Synthesis Plugin

A professional granular synthesis VST3/AU plugin built with JUCE.

## Current Status

✅ **Phase 0.1 - Hello World Plugin** (Completed)
- JUCE project setup with CMake
- Basic plugin shell with UI
- VST3 and AU builds working
- Audio passthrough implemented
- Successfully builds and installs to system plugin directories

## Installation

The plugin is automatically installed to:
- **macOS VST3**: `~/Library/Audio/Plug-Ins/VST3/Grains.vst3`
- **macOS AU**: `~/Library/Audio/Plug-Ins/Components/Grains.component`

## Testing the Plugin

1. Open your DAW (Ableton, Logic, Reaper, etc.)
2. Rescan plugins or restart the DAW
3. Look for "Grains" in your plugin list
4. Load it on an audio track
5. You should see a dark window with "Grains VST" and "Plugin Loaded Successfully!"
6. Currently it just passes audio through unchanged (bypass mode)

## Building from Source

### Prerequisites
- macOS with Xcode Command Line Tools
- CMake 3.22 or higher
- JUCE Framework (currently at `/Users/colton/Downloads/JUCE`)

### Build Commands
```bash
cd /Users/colton/code/me/plugins/grains-vst
mkdir build
cd build
cmake ..
cmake --build . --config Release -j4
```

## Project Structure

```
grains-vst/
├── CMakeLists.txt           # Build configuration
├── Source/
│   ├── PluginProcessor.h    # Audio processing logic
│   ├── PluginProcessor.cpp
│   ├── PluginEditor.h       # UI/GUI code
│   └── PluginEditor.cpp
├── build/                   # Build artifacts (gitignored)
└── README.md
```

## Roadmap

### Phase 1: MVP - Core Granular Engine (4-6 weeks)
- [ ] Sample loading (drag & drop WAV/AIFF)
- [ ] Position control
- [ ] Grain size control
- [ ] Density control
- [ ] Spray/randomization
- [ ] Pitch shifting
- [ ] Basic waveform display
- [ ] Volume/gain control
- [ ] Dry/wet mix

### Phase 2: Enhanced Controls (3-4 weeks)
- [ ] Stereo width control
- [ ] Pan randomization
- [ ] Pitch randomization
- [ ] Freeze mode
- [ ] Reverse grains
- [ ] 4x LFO modulation matrix
- [ ] Animated waveform with grain visualization
- [ ] Preset system

### Phase 3: V2 Features (4-6 weeks)
- [ ] Envelope shape selection
- [ ] Granular delay mode
- [ ] Multi-sample support
- [ ] MIDI triggering
- [ ] Macro controls
- [ ] Factory preset library

## Development Notes

- Using JUCE 7.x
- Targeting C++17
- Building VST3 and AU formats
- Dark UI aesthetic matching web version (#0a0a0a background)
- Plugin currently passes audio through unchanged (good for testing)

## Next Steps

1. Test loading in multiple DAWs
2. Implement basic parameter system (AudioProcessorValueTreeState)
3. Start building grain voice class
4. Implement sample loading functionality

---

**Project Start Date**: October 27, 2025
**Target MVP Completion**: December 2025
**Commercial Release**: Q1 2026
