#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "GrainVoice.h"
#include "Envelope.h"
#include <vector>
#include <random>
#include <array>

//==============================================================================
/**
 * GrainEngine - Manages grain generation and playback
 *
 * Responsibilities:
 * - Schedule new grains based on density parameter
 * - Manage a pool of GrainVoice objects
 * - Mix active grains together
 * - Apply spray (randomization) to grain parameters
 */
class GrainEngine
{
public:
    GrainEngine()
    {
        // Initialize grain voice pool (max 256 simultaneous grains)
        // Increased from 64 to handle extreme polyphonic density settings
        // Reserve capacity to avoid reallocations
        grainVoices.resize(256);
        grainVoices.shrink_to_fit(); // Ensure contiguous memory for cache efficiency

        // Initialize random number generator
        randomEngine.seed(std::random_device{}());

        // Pre-allocate random distributions to avoid construction overhead
        sprayDistribution = std::uniform_real_distribution<float>(-1.0f, 1.0f);
        pitchDistribution = std::uniform_real_distribution<float>(-1.0f, 1.0f);
        panDistribution = std::uniform_real_distribution<float>(-1.0f, 1.0f);
        reverseDistribution = std::uniform_real_distribution<float>(0.0f, 100.0f);
        octaveDistribution = std::uniform_real_distribution<float>(0.0f, 100.0f);
        filterDistribution = std::uniform_real_distribution<float>(0.0f, 1.0f);
        detuneDistribution = std::uniform_real_distribution<float>(-1.0f, 1.0f);
        sizeDistribution = std::uniform_real_distribution<float>(-1.0f, 1.0f);
    }

    //==============================================================================
    /** Prepare for playback */
    void prepare(double sampleRate, int samplesPerBlock)
    {
        juce::ignoreUnused(samplesPerBlock);
        currentSampleRate = sampleRate;
        samplesToNextGrain = 0.0;

        // Set sample rate for all envelopes (the array is fixed-size, no allocations)
        for (auto& env : envelopes)
            env.setSampleRate(sampleRate);
    }

    /** Set envelope parameters */
    void setEnvelopeAttack(float timeMs) { envelopeAttack = timeMs; }
    void setEnvelopeDecay(float timeMs) { envelopeDecay = timeMs; }
    void setEnvelopeSustain(float level01) { envelopeSustain = level01; }
    void setEnvelopeRelease(float timeMs) { envelopeRelease = timeMs; }

    /** Set the source audio buffer (thread-safe) */
    void setSourceBuffer(const juce::AudioBuffer<float>& buffer)
    {
        // CRITICAL: Clear all active grains before changing buffer
        // This prevents audio thread from reading invalid memory
        clearAllGrains();
        sourceBuffer = &buffer;
    }

    /** Check if a source sample is loaded */
    bool hasSource() const
    {
        return sourceBuffer != nullptr && sourceBuffer->getNumSamples() > 0;
    }

    /** Clear all active grains (for safe buffer swapping) */
    void clearAllGrains()
    {
        for (auto& voice : grainVoices)
        {
            voice.deactivate();
        }
    }

    //==============================================================================
    /** Note on - start playing grains */
    void noteOn(int midiNote, int velocity)
    {
        juce::ignoreUnused(velocity);  // Could use for volume control later
        if (midiNote < 0 || midiNote >= 128) return;

        if (!activeNoteFlags[midiNote])
        {
            activeNoteFlags[midiNote] = true;
            ++activeNoteCount;
        }

        // Set the current note for pitch tracking when this is the only active note
        if (activeNoteCount == 1)
            currentMidiNote = midiNote;

        // Trigger this note's envelope (array slot, no allocation)
        auto& envelope = envelopes[midiNote];
        envelope.setSampleRate(currentSampleRate);
        envelope.setAttack(envelopeAttack);
        envelope.setDecay(envelopeDecay);
        envelope.setSustain(envelopeSustain);
        envelope.setRelease(envelopeRelease);
        envelope.noteOn();
    }

    /** Note off - trigger envelope release (grains continue during release) */
    void noteOff(int midiNote)
    {
        if (midiNote < 0 || midiNote >= 128) return;

        // DON'T clear activeNoteFlags yet — keep generating grains during release!
        // Note will be removed when envelope becomes Idle (see cleanup in process())
        envelopes[midiNote].noteOff();
    }

    /** Check if any notes are active */
    bool hasActiveNotes() const
    {
        return activeNoteCount > 0;
    }

    /** Get pitch offset in semitones from MIDI note (C4 = 60 = no offset) */
    float getMidiPitchOffset() const
    {
        return static_cast<float>(currentMidiNote - 60);  // C4 (middle C) as reference
    }

    /** Get grain voices for visualization */
    const std::vector<GrainVoice>& getVoices() const
    {
        return grainVoices;
    }

    /** Get current position (for visualization) - returns the ACTUAL position, not frozen */
    float getCurrentPosition() const { return lastActualPosition; }

    /** Get spray amount (for visualization) */
    float getCurrentSpray() const { return lastSpray; }

    /** Check if freeze mode is active */
    bool isFrozen() const { return wasFrozen; }

    //==============================================================================
    /** Process a block of audio */
    void process(juce::AudioBuffer<float>& outputBuffer,
                 float position01,           // 0-1: position in sample
                 float sprayPercent,         // 0-100: randomization amount
                 float grainSizeMs,          // 5-500: grain duration in ms
                 float densityGrainsPerSec,  // 1-100: grains per second
                 float pitchSemitones,       // -24 to +24: pitch shift
                 float pitchSpreadSemitones, // 0-24: pitch randomization
                 float panPosition,          // -1 to +1: stereo position
                 float panSpreadPercent,     // 0-100: pan randomization
                 bool freezeMode,            // true: lock position
                 float reversePercent,       // 0-100: reverse probability
                 float timeStretch,          // 0.25-4: time stretch multiplier
                 int grainShape,             // 0-4: grain envelope shape
                 float octaveSpread,         // 0-3: octave shift range
                 float octaveProbability,    // 0-100: probability of octave shift
                 float thirdOctaveProb = 0.0f,      // 0-100: probability of 3rd octave
                 float filterRandomization = 0.0f,  // 0-100: per-grain filter cutoff randomization
                 float detuneCents = 0.0f,          // 0-50: per-grain micro-detuning in cents
                 float jitterPercent = 0.0f,        // 0-100: timing humanization
                 float grainSizeRandomization = 0.0f) // 0-100: per-grain size variation
    {
        if (!hasSource())
            return;

        const int numSamples = outputBuffer.getNumSamples();
        const int numChannels = outputBuffer.getNumChannels();

        // Get direct buffer pointers for faster writes (avoids addSample overhead)
        float* leftOutputPtr = outputBuffer.getWritePointer(0);
        float* rightOutputPtr = numChannels >= 2 ? outputBuffer.getWritePointer(1) : leftOutputPtr;

        // Calculate base density (samples between grain starts)
        double baseSamplesPerGrain = currentSampleRate / densityGrainsPerSec;

        // Store freeze position on first freeze activation
        if (freezeMode && !wasFrozen)
        {
            frozenPosition = position01;
            wasFrozen = true;
        }
        else if (!freezeMode)
        {
            wasFrozen = false;
            // Reset frozen position when freeze is disabled to avoid stale state
            frozenPosition = position01;
        }

        // Use frozen position if freeze is active, otherwise use current position
        float actualPosition = (freezeMode && wasFrozen) ? frozenPosition : position01;

        // Store for visualization - keep track of both actual and used position
        lastActualPosition = position01;  // Always store the actual position parameter
        lastUsedPosition = actualPosition;  // The position actually used for grains
        lastSpray = sprayPercent;

        // Update tail time for all active envelopes based on current grain size and time stretch
        // Time stretch > 1 = slower playback (stretched in time) → grains take longer to complete
        // Time stretch < 1 = faster playback (compressed) → grains finish sooner
        // Tail duration = grainSizeMs * timeStretch (to account for playback speed)
        float tailDuration = grainSizeMs * timeStretch;
        for (int note = 0; note < 128; ++note)
            if (activeNoteFlags[note])
                envelopes[note].setTailTime(tailDuration);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Generate grains during Attack, Decay, Sustain, AND Release
            // STOP generating only during Tail (after release ends) to let final grains finish
            if (hasActiveNotes())
            {
                // Check if any notes are NOT in tail phase (still generating)
                bool shouldGenerateGrains = false;
                for (int note = 0; note < 128; ++note)
                {
                    if (!activeNoteFlags[note]) continue;
                    const auto state = envelopes[note].getState();
                    if (state != Envelope::Tail && state != Envelope::Idle)
                    {
                        shouldGenerateGrains = true;
                        break;
                    }
                }

                // Generate grains for notes that are still active (not in tail/idle)
                if (shouldGenerateGrains && samplesToNextGrain <= 0.0)
                {
                    for (int midiNote = 0; midiNote < 128; ++midiNote)
                    {
                        if (!activeNoteFlags[midiNote]) continue;
                        const auto state = envelopes[midiNote].getState();
                        if (state != Envelope::Tail && state != Envelope::Idle)
                        {
                            float notePitchOffset = static_cast<float>(midiNote - 60);
                            float totalPitch = pitchSemitones + notePitchOffset;

                            startNewGrain(actualPosition, sprayPercent, grainSizeMs,
                                         totalPitch, pitchSpreadSemitones,
                                         panPosition, panSpreadPercent, reversePercent,
                                         grainShape, octaveSpread, octaveProbability,
                                         thirdOctaveProb, midiNote, filterRandomization, detuneCents,
                                         grainSizeRandomization);
                        }
                    }

                    // Apply jitter: random timing offset (±jitterPercent of grain interval)
                    float jitterAmount = 0.0f;
                    if (jitterPercent > 0.0f)
                    {
                        float jitterRange = (jitterPercent / 100.0f) * static_cast<float>(baseSamplesPerGrain);
                        jitterAmount = (sprayDistribution(randomEngine) * jitterRange);
                    }

                    samplesToNextGrain += baseSamplesPerGrain + jitterAmount;
                }

                if (shouldGenerateGrains)
                    samplesToNextGrain -= 1.0;
            }

            // Advance per-note envelopes once per sample, and write the value to a
            // sparse lookup table indexed by MIDI note. Only active notes touch the
            // table — at typical 1–3-note polyphony this replaces 128 slot ops/sample
            // with effectively 0.
            for (int note = 0; note < 128; ++note)
                if (activeNoteFlags[note])
                    envValuesThisSample[note] = envelopes[note].getNextValue();

            // Single pass over the voice pool: each voice's output is scaled by its
            // own note's envelope value (computed above) and summed into the master.
            // Voices whose note is no longer active contribute nothing (env stays at 0).
            float leftSample = 0.0f;
            float rightSample = 0.0f;

            for (auto& voice : grainVoices)
            {
                if (!voice.activeVoice())
                    continue;

                auto [left, right] = voice.getNextSample();
                const int note = voice.getMidiNote();

                if (note >= 0 && note < 128 && envelopes[note].isActive())
                {
                    const float env = envValuesThisSample[note];
                    leftSample  += left  * env;
                    rightSample += right * env;
                }
            }

            // Write to output buffer using direct pointer access
            if (numChannels >= 2)
            {
                leftOutputPtr[sample]  += leftSample;
                rightOutputPtr[sample] += rightSample;
            }
            else
            {
                // Mono: average left and right
                leftOutputPtr[sample] += (leftSample + rightSample) * 0.5f;
            }
        }

        // Clean up notes whose envelope has fully finished. One linear scan over
        // the flag array — same complexity as a std::set walk, no node dealloc.
        bool anyRemoved = false;
        for (int note = 0; note < 128; ++note)
        {
            if (activeNoteFlags[note] && envelopes[note].getState() == Envelope::Idle)
            {
                activeNoteFlags[note] = false;
                --activeNoteCount;
                anyRemoved = true;
            }
        }

        // If we removed something, refresh currentMidiNote to the lowest still-active note.
        if (anyRemoved && activeNoteCount > 0)
        {
            for (int note = 0; note < 128; ++note)
            {
                if (activeNoteFlags[note]) { currentMidiNote = note; break; }
            }
        }
    }

private:
    //==============================================================================
    /** Start a new grain with the given parameters (optimized - reuse distributions) */
    void startNewGrain(float position01, float sprayPercent, float grainSizeMs,
                       float pitchSemitones, float pitchSpreadSemitones,
                       float panPosition, float panSpreadPercent, float reversePercent,
                       int grainShape,
                       float octaveSpread, float octaveProbability,
                       float thirdOctaveProb = 0.0f, int midiNote = -1,
                       float filterRandomization = 0.0f, float detuneCents = 0.0f,
                       float grainSizeRandomization = 0.0f)
    {
        // Find an inactive voice
        GrainVoice* voice = findFreeVoice();
        if (voice == nullptr)
            return;  // No free voices available

        // Apply spray (randomization) to position - reuse pre-allocated distribution
        const float spray01 = sprayPercent * 0.01f; // Multiply instead of divide (faster)
        const float randomizedPosition = juce::jlimit(0.0f, 1.0f,
            position01 + spray01 * sprayDistribution(randomEngine));

        // Apply pitch randomization
        float randomizedPitch = pitchSemitones +
            pitchSpreadSemitones * pitchDistribution(randomEngine);

        // Apply octave randomization (with probability)
        if (octaveProbability > 0.0f && octaveSpread > 0.0f)
        {
            float octaveRoll = octaveDistribution(randomEngine);  // 0-100
            if (octaveRoll < octaveProbability)
            {
                int semitoneShift = 0;

                // Octave shift logic
                int octaves = 0;

                // Special handling for 3rd octave (lower probability)
                if (octaveSpread >= 3.0f && thirdOctaveProb > 0.0f)
                {
                    float thirdOctaveRoll = octaveDistribution(randomEngine);  // 0-100
                    if (thirdOctaveRoll < thirdOctaveProb)
                    {
                        // Pick 3rd octave (±3)
                        octaves = (octaveDistribution(randomEngine) < 50.0f) ? -3 : 3;
                    }
                    else
                    {
                        // Pick from ±1 or ±2 octaves
                        float innerRoll = octaveDistribution(randomEngine) / 100.0f;  // 0-1
                        int magnitude = (innerRoll < 0.5f) ? 1 : 2;
                        octaves = (octaveDistribution(randomEngine) < 50.0f) ? -magnitude : magnitude;
                    }
                }
                else
                {
                    // Standard behavior: random octave shift within range
                    float octaveShift = (octaveDistribution(randomEngine) / 100.0f * 2.0f - 1.0f) * octaveSpread;
                    octaves = static_cast<int>(std::round(octaveShift));
                }

                // Convert to semitones (1 octave = 12 semitones)
                semitoneShift = octaves * 12;

                randomizedPitch += static_cast<float>(semitoneShift);
            }
        }

        // Apply micro-detuning (cents to semitones: divide by 100)
        if (detuneCents > 0.0f)
        {
            float detuneAmount = detuneDistribution(randomEngine) * detuneCents / 100.0f;
            randomizedPitch += detuneAmount;
        }

        // Apply pan randomization
        const float panSpread01 = panSpreadPercent * 0.01f;
        const float randomizedPan = juce::jlimit(-1.0f, 1.0f,
            panPosition + panSpread01 * panDistribution(randomEngine));

        // Determine if this grain should be reversed
        const bool shouldReverse = reverseDistribution(randomEngine) < reversePercent;

        // Calculate per-grain filter cutoff with randomization
        // Use logarithmic distribution for more musical spread
        // filterRandomization 0-100 controls how much variation
        float filterCutoff = 20000.0f;  // Default: fully open
        if (filterRandomization > 0.0f)
        {
            // Random value 0-1
            float randomVal = filterDistribution(randomEngine);

            // Perceptual range: 200Hz to 20kHz
            // Use squared curve to approximate logarithmic distribution (avoids log/exp)
            float minFreq = 200.0f;
            float maxFreq = 20000.0f;

            // Interpolate between no randomization and full randomization
            float randomAmount = filterRandomization * 0.01f;
            float effectiveMin = maxFreq - randomAmount * (maxFreq - minFreq);

            // Squared curve approximates perceptual (log) distribution
            // Low values cluster toward the minimum frequency
            float curved = randomVal * randomVal;
            filterCutoff = effectiveMin + curved * (maxFreq - effectiveMin);
        }

        // Apply grain size randomization (per-grain variation)
        float randomizedGrainSize = grainSizeMs;
        if (grainSizeRandomization > 0.0f)
        {
            // Random variation: ±grainSizeRandomization%
            // Example: at 50% randomization, grain can be 50-150% of base size
            float randomVal = sizeDistribution(randomEngine);  // -1 to +1
            float variation = randomVal * (grainSizeRandomization / 100.0f);
            randomizedGrainSize = grainSizeMs * (1.0f + variation);
            // Clamp to valid range
            randomizedGrainSize = juce::jlimit(5.0f, 2000.0f, randomizedGrainSize);
        }

        // Calculate stereo spread (Haas effect for spatial width)
        // Use pan spread as base, apply random offset per grain
        // Typical Haas delays: 5-30ms for width without losing center
        float stereoSpreadMs = (panSpreadPercent / 100.0f) * 15.0f;  // Max 15ms spread
        float randomSpreadOffset = panDistribution(randomEngine);     // -1 to +1
        float finalSpreadMs = stereoSpreadMs * randomSpreadOffset;
        float stereoSpreadSamples = (finalSpreadMs / 1000.0f) * static_cast<float>(currentSampleRate);

        // Start the grain with all parameters
        voice->start(*sourceBuffer, randomizedPosition, randomizedGrainSize, randomizedPitch,
                     randomizedPan, shouldReverse, currentSampleRate, grainShape, midiNote, filterCutoff,
                     stereoSpreadSamples);
    }

    /** Find an inactive grain voice, or steal the oldest one (optimized) */
    GrainVoice* findFreeVoice()
    {
        // Round-robin voice allocation (faster than searching, more even distribution)
        // Start from last allocated position for better cache locality
        const int numVoices = static_cast<int>(grainVoices.size());

        for (int i = 0; i < numVoices; ++i)
        {
            lastVoiceIndex = (lastVoiceIndex + 1) % numVoices;
            if (!grainVoices[lastVoiceIndex].activeVoice())
                return &grainVoices[lastVoiceIndex];
        }

        // All voices active - steal using round-robin (fair, predictable)
        lastVoiceIndex = (lastVoiceIndex + 1) % numVoices;
        return &grainVoices[lastVoiceIndex];
    }

    //==============================================================================
    // State
    std::vector<GrainVoice> grainVoices;
    const juce::AudioBuffer<float>* sourceBuffer = nullptr;
    std::mt19937 randomEngine;

    // Pre-allocated random distributions (avoid construction overhead)
    std::uniform_real_distribution<float> sprayDistribution;
    std::uniform_real_distribution<float> pitchDistribution;
    std::uniform_real_distribution<float> panDistribution;
    std::uniform_real_distribution<float> reverseDistribution;
    std::uniform_real_distribution<float> octaveDistribution;
    std::uniform_real_distribution<float> filterDistribution;
    std::uniform_real_distribution<float> detuneDistribution;
    std::uniform_real_distribution<float> sizeDistribution;

    double currentSampleRate = 44100.0;
    double samplesToNextGrain = 0.0;

    // Round-robin voice allocation
    int lastVoiceIndex = 0;

    // Freeze mode state
    bool wasFrozen = false;
    float frozenPosition = 0.5f;

    // MIDI note tracking. Flag-array + count replaces std::set<int> so that
    // noteOn (called from the MIDI loop in processBlock) doesn't allocate a
    // tree node on the audio thread. Iteration order matches the old set
    // (ascending MIDI note) because we always scan 0..127.
    std::array<bool, 128> activeNoteFlags {};
    int activeNoteCount = 0;
    int currentMidiNote = 60;  // Middle C as default

    // Per-note envelope system. std::array indexed by MIDI note (0..127) — fixed
    // storage, no allocations on note-on. envValuesThisSample is a sparse lookup
    // populated each sample for active notes only.
    std::array<Envelope, 128> envelopes;
    std::array<float, 128> envValuesThisSample {};
    float envelopeAttack = 10.0f;
    float envelopeDecay = 300.0f;
    float envelopeSustain = 0.8f;  // 0-1 range
    float envelopeRelease = 500.0f;

    // Visualization state
    float lastActualPosition = 0.5f;  // The position parameter value (for UI display)
    float lastUsedPosition = 0.5f;    // The position actually used for grains (frozen or not)
    float lastSpray = 0.0f;
};
