# Somnia Grains VST - Phase 3 Development Plan

## Overview
Phase 3 focuses on independent time/pitch manipulation and advanced modulation capabilities to create more expressive and unique granular textures.

---

## 🎯 Core Features

### 1. Independent Pitch/Time Control
**Problem**: Currently, changing pitch also changes grain playback speed (time-domain pitch shifting).

**Solution**: Implement two independent controls:

#### A. Time Stretch Parameter (0.25x - 4x)
- **What it does**: Changes grain playback speed WITHOUT affecting pitch
- **Implementation**: Resample grains using interpolation (linear or cubic)
- **Use case**: Slow down/speed up texture while maintaining original pitch
- **Parameter range**:
  - 0.25x = 4x slower (1/4 speed)
  - 1.0x = normal speed
  - 4.0x = 4x faster

#### B. Pitch Shift Parameter (remains as-is: -24 to +24 semitones)
- **What it does**: Changes pitch WITHOUT affecting grain duration
- **Implementation**: Combine with time stretch to cancel out time effects
  - Formula: `playbackRate = timeStretch * pitchRatio`
  - Where `pitchRatio = 2^(semitones/12)`
- **Use case**: Transpose texture without changing timing

**Example**:
- User wants +12 semitones (octave up) with no speed change:
  - `pitchRatio = 2^(12/12) = 2.0` (would normally be 2x faster)
  - `timeStretch = 0.5` (to compensate)
  - `finalPlaybackRate = 2.0 * 0.5 = 1.0` (grain plays at normal speed but pitched up!)

---

### 2. Advanced Modulation System
**Goal**: Allow parameters to evolve over time for dynamic, evolving textures.

#### LFO System (Low Frequency Oscillators)
- **Modulation targets**: Position, Spray, Grain Size, Density, Pitch, Pan, Pan Spread, Reverse
- **Waveforms**: Sine, Triangle, Saw Up, Saw Down, Square, Random (sample & hold)
- **Controls per LFO**:
  - Rate: 0.01 Hz - 20 Hz (with tempo sync option)
  - Depth: 0-100% (how much modulation affects parameter)
  - Phase: 0-360° (offset for multiple LFOs)

#### Number of LFOs
- **Decision**: 4 LFOs for maximum creative flexibility
- **UI Strategy**: Hide in collapsible "Advanced Modulation" panel to avoid overwhelming basic users
- **Tempo Sync**: Each LFO has individual tempo sync toggle (syncs to DAW BPM)

---

### 3. Envelope System
**Goal**: Shape grain parameters over time based on note duration.

#### Envelope Generator
- **Type**: ADSR (Attack, Decay, Sustain, Release)
- **Targets**: Same as LFO targets
- **Behavior**:
  - Triggered by MIDI note on
  - Attack: ramps up from 0 to peak
  - Decay: falls to sustain level
  - Sustain: holds while note is held
  - Release: falls to 0 after note off

#### Use Cases
- Gradually increase density as note is held
- Fade in spray amount on attack
- Sweep position during release

---

### 4. Grain Envelope Shapes
**Current**: Hann window (smooth bell curve)

**Enhancement**: Add multiple envelope options:
- **Hann** (current): Smooth, no clicks
- **Triangle**: Sharper attack/release
- **Trapezoid**: Sustain portion in middle
- **Exponential**: Natural decay feel
- **Gaussian**: Very smooth, natural

**UI**: Dropdown or visual selector showing envelope shapes

---

## 🎨 UI/UX Design Suggestions

### Layout Architecture

```
┌─────────────────────────────────────────────────────────────┐
│  SOMNIA                        [Sample: filename.wav] [Load] │
├─────────────────────────────────────────────────────────────┤
│                                                               │
│  ┌────────────────────────────────────────────────────────┐ │
│  │                                                          │ │
│  │         Unified Visualization (Waveform + Grains)       │ │
│  │                                                          │ │
│  └────────────────────────────────────────────────────────┘ │
│                                                               │
├─────────────────────────────────────────────────────────────┤
│  GRAIN CONTROLS                                               │
│  ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐    │
│  │ Pos  │ │Spray │ │ Size │ │Densty│ │ Pitch│ │ Time │    │
│  │      │ │      │ │      │ │      │ │      │ │Stretch│   │
│  └──────┘ └──────┘ └──────┘ └──────┘ └──────┘ └──────┘    │
│  ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐ [Envelope:▼]          │
│  │ Pan  │ │ Gain │ │PanSpd│ │Revrse│                        │
│  └──────┘ └──────┘ └──────┘ └──────┘ [Freeze]               │
├─────────────────────────────────────────────────────────────┤
│  MODULATION                                                   │
│  ┌─────────────────────────┐  ┌─────────────────────────┐  │
│  │ LFO 1                   │  │ LFO 2                   │  │
│  │ Rate: [──●────]  2.5 Hz │  │ Rate: [───●───]  0.8 Hz │  │
│  │ Wave: [Sine ▼]          │  │ Wave: [Random ▼]        │  │
│  │ Target: [Position ▼]    │  │ Target: [Spray ▼]       │  │
│  │ Depth: [────●──] 50%    │  │ Depth: [───●───] 60%    │  │
│  └─────────────────────────┘  └─────────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

---

### Interactive Visualization Enhancements

#### 1. LFO Visualization on Waveform
- **Show active modulation**: When LFO targets Position, draw animated overlay
- **Color coding**:
  - LFO 1 = soft blue glow
  - LFO 2 = soft orange glow
- **Movement**: Animate the window area moving with LFO

#### 2. Time Stretch Indicator
- **Visual cue**: Show grain "trails" or motion blur when time stretch is active
- **Slower than normal**: Grains appear to move in slow motion
- **Faster than normal**: Motion blur or multiple grains at once

#### 3. Modulation Amount Display
- **On hover**: Show which parameters are being modulated
- **Visual feedback**: Pulse or glow around knobs being modulated

---

### Color System (Somnia Theme)

#### Current Palette
- Midnight Blue: `#1a2332` (background)
- Soft Lavender: `#b4a5d8` (primary accent)
- Dusty Rose: `#d4a5a5` (secondary accent)
- Periwinkle: `#9ba4d8` (text)
- Cream: `#f5f1e8` (highlights)
- Warm Charcoal: `#4a4a52` (borders)

#### New Additions for Phase 3
- **LFO 1**: `#8ab4d8` (Sky Blue) - for LFO 1 indicators
- **LFO 2**: `#d8b48a` (Warm Amber) - for LFO 2 indicators
- **Envelope**: `#a8d8a5` (Soft Green) - for envelope modulation
- **Warning/Active**: `#d88a8a` (Coral) - for clipping/limiting indicators

---

### Interaction Design Patterns

#### 1. Modulation Assignment
**Method A - Drag and Drop**:
- Drag from LFO "output" dot to parameter knob
- Visual connection line appears
- Multiple targets allowed per LFO

**Method B - Dropdown (Simpler)**:
- Dropdown menu on LFO shows all parameters
- Select target from list
- Show small indicator on target knob

**Recommendation**: Start with Method B (simpler to implement)

#### 2. Time Stretch Control
**Option A - Link Toggle**:
```
Pitch:  [──●──] +5 st    Time: [──●──] 1.0x
        [🔗 Link]  ← Toggle locks them together (current behavior)
```
When unlinked, both can be adjusted independently.

**Option B - Separate Always**:
Just have two independent controls, no link toggle.

**Recommendation**: Option A - Link toggle (backward compatible)

#### 3. Envelope Visualization
**In modulation section**:
```
┌──────────────────────────┐
│ ENVELOPE                 │
│ Target: [Density ▼]      │
│                          │
│      ╱─────╲             │
│     ╱       ╲            │
│    ╱         ╲___        │
│   ╱              ╲       │
│  A  D  S     R           │
│ [●][●][●]   [●]          │
└──────────────────────────┘
```
Visual ADSR curve with draggable handles.

---

## 📋 Revised Implementation Phases

Since all features are important, we'll break them into separate focused phases for quality:

---

## **PHASE 3: Time Stretch & Grain Shapes**
**Goal**: Independent pitch/time control + grain character variations
**Timeline**: 2-3 weeks

### Tasks:
- [ ] Add timeStretch parameter (0.25x - 4x)
- [ ] Modify GrainVoice playback to support independent speed via interpolation
- [ ] Update pitch calculation formula: `playbackRate = timeStretch * 2^(semitones/12)`
- [ ] Add Time Stretch knob to UI (next to Pitch)
- [ ] Add link toggle button between Pitch and Time Stretch
- [ ] Implement multiple grain envelope shapes (Hann, Triangle, Trapezoid, Exponential, Gaussian)
- [ ] Add grain shape selector dropdown to UI
- [ ] Test all combinations of pitch/time/shape

**Deliverables**:
- ✅ Pitch and time are independently controllable
- ✅ 5 different grain envelope shapes available
- ✅ Link toggle preserves backward-compatible behavior

**Why together?** Both modify grain playback at the voice level - efficient to implement together.

---

## **PHASE 4: LFO Modulation System**
**Goal**: Dynamic, evolving textures through automated parameter modulation
**Timeline**: 3-4 weeks

### Tasks:
- [ ] Create LFO class with waveform generation (Sine, Triangle, Saw Up, Saw Down, Square, Random)
- [ ] Implement tempo sync system (reads DAW BPM)
- [ ] Add 4 LFO instances to processor
- [ ] Create collapsible "Advanced Modulation" panel in UI
- [ ] Design and implement LFO control layout (4 LFO sections)
- [ ] Each LFO has: Rate, Depth, Waveform selector, Target selector, Tempo Sync toggle
- [ ] Implement modulation routing system (LFO → parameter)
- [ ] Add modulation amount indicators on target knobs
- [ ] Add optional LFO visualization overlay on waveform
- [ ] Test with various waveforms and targets

**Deliverables**:
- ✅ 4 independent LFOs
- ✅ Each LFO can modulate any parameter
- ✅ Individual tempo sync toggles per LFO
- ✅ Clean, collapsible UI that doesn't overwhelm
- ✅ Visual feedback showing active modulation

**Why separate?** Modulation system is complex - requires careful UI design and thorough testing. Keeping it focused ensures quality.

---

## **PHASE 5: Envelope System**
**Goal**: Expressive, note-based parameter control
**Timeline**: 2-3 weeks

### Tasks:
- [ ] Create ADSR envelope class
- [ ] Hook envelope to MIDI note on/off events
- [ ] Add envelope section to modulation panel
- [ ] Design visual ADSR curve editor
- [ ] Implement draggable ADSR handles
- [ ] Add envelope target selector (which parameter to modulate)
- [ ] Add envelope depth control
- [ ] Test envelope timing with various ADSR settings
- [ ] Add envelope amount indicator on target knobs
- [ ] Test interaction between LFOs and envelope on same parameter

**Deliverables**:
- ✅ 1 ADSR envelope generator
- ✅ Responds to MIDI note on/off
- ✅ Visual curve editor
- ✅ Can modulate any parameter
- ✅ Works alongside LFO modulation

**Why separate?** Envelope system requires MIDI integration and careful timing - better to focus on it independently after LFO system is stable.

---

## **PHASE 6: Polish, Optimization & Advanced Features**
**Goal**: Performance optimization, preset system, final polish
**Timeline**: 2 weeks

### Tasks:
- [ ] Performance profiling and optimization
- [ ] Implement preset save/load system
- [ ] Add preset browser
- [ ] Modulation visualization polish
- [ ] Add tooltips and help text
- [ ] CPU usage optimization
- [ ] User testing and feedback
- [ ] Bug fixes
- [ ] Documentation updates

**Deliverables**:
- ✅ Stable, optimized performance
- ✅ Preset system for saving/loading patches
- ✅ Polished, professional UI
- ✅ Comprehensive testing

---

## 🎯 Summary Timeline

| Phase | Focus | Duration | Total |
|-------|-------|----------|-------|
| Phase 3 | Time Stretch + Grain Shapes | 2-3 weeks | 3 weeks |
| Phase 4 | LFO System (4 LFOs) | 3-4 weeks | 7 weeks |
| Phase 5 | Envelope System | 2-3 weeks | 10 weeks |
| Phase 6 | Polish & Presets | 2 weeks | 12 weeks |

**Total estimated time**: ~12 weeks for all features

**Approach**: Ship each phase as a usable update. Users get features progressively rather than waiting for everything.

---

## 🔧 Technical Considerations

### 1. Time Stretching Algorithm
**Simple approach** (start here):
- Linear interpolation for playback rate adjustment
- Works well for small time stretch ratios (0.5x - 2x)

**Advanced approach** (if needed):
- Cubic interpolation for better quality
- Sinc interpolation for best quality (slower)

### 2. LFO Performance
- Pre-calculate waveform tables (sine, saw, etc.)
- Use linear interpolation between samples
- Update at control rate (e.g., every 32 samples) not audio rate

### 3. Modulation Smoothing
- Add smoothing to prevent zipper noise
- Use one-pole lowpass filter: `y = y + alpha * (target - y)`
- Alpha ≈ 0.01 for smooth modulation

### 4. CPU Optimization
- Phase 3 adds complexity - monitor CPU usage
- Consider grain count limit (currently 64 voices)
- Profile hot paths (LFO updates, envelope calculations)

---

## 🎵 Musical Use Cases

### Time Stretch Examples
1. **Ambient Pads**: Slow time stretch (0.5x) + low density = stretched, dreamy textures
2. **Rhythmic Glitches**: Fast time stretch (2x-4x) + high density = rapid, glitchy sounds
3. **Pitched Drones**: Pitch up +12st, time stretch 0.5x = high drone at normal speed

### LFO Examples
1. **Wandering Position**: LFO 1 → Position (slow sine) = sample exploration
2. **Rhythmic Density**: LFO 2 → Density (square wave, tempo-synced) = rhythmic pulses
3. **Stereo Movement**: LFO 1 → Pan (triangle) = auto-panning texture

### Envelope Examples
1. **Swelling Clouds**: Envelope → Density (slow attack/release) = gradual texture build
2. **Spray Fade-In**: Envelope → Spray (fast attack) = clean start, chaotic sustain
3. **Position Sweep**: Envelope → Position = scan through sample on each note

---

## 🎨 Alternative UI Concepts

### Concept A: Tabbed Interface
```
[Grains] [Modulation] [Effects]
```
- Separate sections into tabs
- More space per section
- Clean, organized

**Pros**: More room for controls
**Cons**: Hidden functionality

---

### Concept B: Collapsible Sections
```
▼ GRAIN CONTROLS
  [knobs visible]

▼ MODULATION
  [LFO controls visible]

▶ EFFECTS (collapsed)
```
- Accordion-style UI
- Show/hide sections as needed

**Pros**: Flexible, discoverable
**Cons**: Requires more clicking

---

### Concept C: Single Page (Recommended)
- Keep everything visible
- Tighter spacing
- Use 3-4 rows of controls

**Pros**: No hidden controls, fast workflow
**Cons**: Might feel cramped

---

## 💡 Feature Ideas for Future Phases

### Phase 4 Possibilities
- Multi-sample support (layer multiple samples)
- Granular effects (reverb, delay, distortion per grain)
- MIDI mapping system (map any CC to any parameter)
- Sequence mode (step sequencer for grain parameters)
- Wavetable mode (use grain position as wavetable position)

### Phase 5 Possibilities
- MPE support (polyphonic expression)
- Sample browser with preview
- Morph between presets
- Spectral effects
- Machine learning-based grain generation

---

## 📝 Decisions Made

1. **Time Stretch Range**: ✅ 0.25x-4x (confirmed good)
2. **LFO Count**: ✅ 4 LFOs (in collapsible advanced panel)
3. **Tempo Sync**: ✅ Yes, individual toggle per LFO
4. **Preset System**: Phase 6 (after core features)
5. **Effects**: Future phase consideration
6. **MIDI Learn**: Future phase consideration

## 📝 Open Questions - RESOLVED

1. **Phase 3 Priority**: ✅ Starting Phase 3 now (Time Stretch + Grain Shapes)
2. **UI Expansion**: ✅ Keep current size (1000x600), use creative collapsible sections
3. **Modulation Limits**: ✅ LFOs can modulate multiple parameters simultaneously

---

## 🎯 Success Metrics

Phase 3 is successful if:
- ✅ Pitch and time are independently controllable
- ✅ LFOs create evolving, dynamic textures
- ✅ Envelopes allow expressive note-based control
- ✅ UI remains intuitive and not overwhelming
- ✅ CPU usage stays reasonable (< 10% for typical use)
- ✅ Creates sounds not possible in Phase 2

---

## 📚 Resources & References

### Time Stretching
- JUCE InterpolatorType (built-in)
- Rubber Band Library (advanced option)
- SoundTouch library (alternative)

### LFO Implementation
- JUCE Oscillator class
- Custom wavetable implementation

### UI Components
- JUCE Slider (for all knobs)
- JUCE ComboBox (for dropdowns)
- Custom Component (for ADSR visualization)

---

**Ready to begin Phase 3?** Let me know which features you'd like to prioritize and any changes to this plan!
