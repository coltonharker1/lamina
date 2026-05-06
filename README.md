# Lamina

A polyphonic granular sample synthesizer for macOS. Drop in any audio file and play it back as a cloud, a swarm, a stretched drone, or a textural pad — each MIDI note triggers its own grain stream with independent envelope, pitch, and articulation.

VST3 and Audio Unit, MIDI in / stereo out.

## What's in it

**Sample engine**
- Drag-and-drop sample loading (WAV / AIFF / etc.)
- 256-voice grain pool with round-robin allocation
- 4-point cubic Hermite interpolation in the sample read
- Per-grain one-pole filter for darkening / shaping individual grains
- Per-grain Haas-effect stereo spread for spatial width
- Thread-safe sample swapping — change sources mid-playback without crashes

**Grain controls**
- **Focus** — playhead position in the source sample (0–100%)
- **Scatter** — random position deviation around Focus
- **Thread** — grain duration, 5–500 ms
- **Cloud** — grain density, 1–100 grains/sec
- **Lift** — pitch shift in semitones, ±24
- **Drift** + **Halo** — stereo position + per-grain pan randomization
- **Exposure** — output level, 0–200%
- **Reverse** — probability that a grain plays backwards
- **Time Stretch** — independent of pitch (0.25× – 4×) when unlocked
- **Freeze** — locks the playhead for an infinite drone
- **Grain Shape** — Hann, Triangle, Trapezoid, Exponential, Gaussian envelopes (lookup-table backed)

**Polyphony & envelope**
- Per-MIDI-note ADSR (attack / decay / sustain / release) with smooth tail fade
- Polyphonic — each held note runs its own grain stream and envelope simultaneously
- Allocation-free hot path: array-backed envelopes, flag-array note tracking

**Modulation**
- 4 independent LFOs, each with 6 waveforms (sine, triangle, saw up, saw down, square, S&H random)
- 8 modulation targets per LFO (position, spray, size, density, pitch, pan, pan spread, reverse)
- Tempo sync to host BPM via JUCE 7+ playhead API
- Phase offset, depth, and rate per LFO

**Filters**
- Independent low-pass and high-pass post the grain mix
- Resonance / Q control on each
- Real-time-safe coefficient updates (no audio-thread heap activity)

**Texture / humanization**
- Octave randomization with separate 3rd-octave probability
- Per-grain micro-detune (cents)
- Timing jitter (humanize the grain clock)
- Per-grain size randomization
- Per-grain filter randomization

## Installation

The plugin is **not signed or notarized**. macOS Gatekeeper will flag it. The steps below get you around it.

### Option 1 — PKG installer (recommended)

1. Download the latest `Lamina-x.y.z.pkg` from [Releases](https://github.com/coltonharker1/lamina/releases).
2. **Right-click** (or Control-click) the `.pkg` → **Open**. Confirm in the dialog.
   - If you double-click instead, macOS will refuse with *"Apple cannot check it for malicious software"*. Right-click → Open is the one-time bypass.
3. Step through the installer and enter your admin password when prompted.
4. Restart your DAW.

### Option 2 — Manual install

If the installer fails, copy the bundles by hand and strip the quarantine attribute:

```bash
# 1. Copy
sudo cp -R Lamina.component /Library/Audio/Plug-Ins/Components/
sudo cp -R Lamina.vst3      /Library/Audio/Plug-Ins/VST3/

# 2. Permissions + ownership
sudo chmod -R 755 /Library/Audio/Plug-Ins/Components/Lamina.component
sudo chmod -R 755 /Library/Audio/Plug-Ins/VST3/Lamina.vst3
sudo chown -R root:wheel /Library/Audio/Plug-Ins/Components/Lamina.component
sudo chown -R root:wheel /Library/Audio/Plug-Ins/VST3/Lamina.vst3

# 3. Strip Gatekeeper quarantine (the critical step)
sudo xattr -rd com.apple.quarantine /Library/Audio/Plug-Ins/Components/Lamina.component
sudo xattr -rd com.apple.quarantine /Library/Audio/Plug-Ins/VST3/Lamina.vst3
```

### Troubleshooting Gatekeeper

If your DAW silently refuses to load the plugin, or you see *"Lamina.component is damaged and can't be opened"*:

```bash
# Nuke all quarantine / extended attrs and re-bless the bundle
sudo xattr -cr /Library/Audio/Plug-Ins/Components/Lamina.component
sudo xattr -cr /Library/Audio/Plug-Ins/VST3/Lamina.vst3
```

If macOS still refuses, open **System Settings → Privacy & Security**, scroll to the bottom, and click **Allow Anyway** next to the Lamina entry that appears after the first load attempt.

### Supported hosts

- **AU**: Logic Pro, GarageBand, MainStage
- **VST3**: Ableton Live, FL Studio, Cubase, Reaper, Bitwig, Studio One, etc.

Lamina is a **MIDI instrument** — load it on an instrument track and feed it MIDI notes. Drag a sample into the editor to get started.

## Building from source

### Requirements

- macOS 10.13 or later
- Xcode Command Line Tools
- CMake 3.22+
- [JUCE](https://juce.com/) (clone separately, alongside this repo)

### Build

The CMakeLists assumes JUCE lives at `../JUCE` relative to this project. Adjust if your layout differs:

```bash
git clone https://github.com/coltonharker1/lamina.git
cd lamina
cmake -B build -S .
cmake --build build --config Release -j
```

Built bundles end up in:

- `build/GrainsVST_artefacts/AU/Lamina.component`
- `build/GrainsVST_artefacts/VST3/Lamina.vst3`

`COPY_PLUGIN_AFTER_BUILD` is on, so a Release build also copies the freshly-built bundles into `~/Library/Audio/Plug-Ins/`. Self-built bundles have no quarantine attribute, so they load directly — the `xattr` dance is only needed for downloaded artifacts.

### Packaging an installer

```bash
./create-installer.sh
```

Produces `installer-output/Lamina-1.0.pkg`.

## Uninstall

```bash
sudo rm -rf /Library/Audio/Plug-Ins/Components/Lamina.component
sudo rm -rf /Library/Audio/Plug-Ins/VST3/Lamina.vst3
```

## License

MIT — see `LICENSE`.

## Credits

Developed by Project Sonaris.
