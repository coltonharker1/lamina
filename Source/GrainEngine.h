#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "GrainVoice.h"
#include "Envelope.h"
#include <vector>
#include <random>
#include <set>
#include <map>

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
    }

    //==============================================================================
    /** Prepare for playback */
    void prepare(double sampleRate, int samplesPerBlock)
    {
        currentSampleRate = sampleRate;
        samplesToNextGrain = 0.0;

        // Set sample rate for all envelopes
        for (auto& pair : noteEnvelopes)
        {
            pair.second.setSampleRate(sampleRate);
        }
    }

    /** Set envelope parameters */
    void setEnvelopeAttack(float timeMs) { envelopeAttack = timeMs; }
    void setEnvelopeDecay(float timeMs) { envelopeDecay = timeMs; }
    void setEnvelopeSustain(float level01) { envelopeSustain = level01; }
    void setEnvelopeRelease(float timeMs) { envelopeRelease = timeMs; }

    /** Set the source audio buffer */
    void setSourceBuffer(const juce::AudioBuffer<float>& buffer)
    {
        sourceBuffer = &buffer;
    }

    /** Check if a source sample is loaded */
    bool hasSource() const
    {
        return sourceBuffer != nullptr && sourceBuffer->getNumSamples() > 0;
    }

    //==============================================================================
    /** Note on - start playing grains */
    void noteOn(int midiNote, int velocity)
    {
        juce::ignoreUnused(velocity);  // Could use for volume control later
        activeNotes.insert(midiNote);

        // Set the current note for pitch tracking
        if (activeNotes.size() == 1)
            currentMidiNote = midiNote;

        // Create and trigger envelope for this note
        auto& envelope = noteEnvelopes[midiNote];
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
        // DON'T remove from activeNotes yet - keep generating grains during release!
        // Note will be removed when envelope becomes inactive (see cleanup in process())

        // Trigger envelope release for this note
        auto it = noteEnvelopes.find(midiNote);
        if (it != noteEnvelopes.end())
        {
            it->second.noteOff();
        }
    }

    /** Check if any notes are active */
    bool hasActiveNotes() const
    {
        return !activeNotes.empty();
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
                 float octaveSpread,         // 0-2: octave shift range
                 float octaveProbability)    // 0-100: probability of octave shift
    {
        if (!hasSource())
            return;

        const int numSamples = outputBuffer.getNumSamples();

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
        for (auto& pair : noteEnvelopes)
        {
            pair.second.setTailTime(tailDuration);
        }

        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Generate grains during Attack, Decay, Sustain, AND Release
            // STOP generating only during Tail (after release ends) to let final grains finish
            if (hasActiveNotes())
            {
                // Check if any notes are NOT in tail phase (still generating)
                bool shouldGenerateGrains = false;
                for (const auto& pair : noteEnvelopes)
                {
                    auto state = pair.second.getState();
                    // Generate during Attack, Decay, Sustain, and Release - stop only at Tail
                    if (state != Envelope::Tail && state != Envelope::Idle)
                    {
                        shouldGenerateGrains = true;
                        break;
                    }
                }

                // Generate grains for notes that are still active (not in tail/idle)
                if (shouldGenerateGrains && samplesToNextGrain <= 0.0)
                {
                    // Start grains for each active note that's still in Attack/Decay/Sustain/Release
                    for (int midiNote : activeNotes)
                    {
                        auto envIt = noteEnvelopes.find(midiNote);
                        if (envIt != noteEnvelopes.end())
                        {
                            auto state = envIt->second.getState();
                            // Generate grains during Attack/Decay/Sustain/Release - stop at Tail
                            if (state != Envelope::Tail && state != Envelope::Idle)
                            {
                                float notePitchOffset = static_cast<float>(midiNote - 60);
                                float totalPitch = pitchSemitones + notePitchOffset;

                                startNewGrain(actualPosition, sprayPercent, grainSizeMs,
                                             totalPitch, pitchSpreadSemitones,
                                             panPosition, panSpreadPercent, reversePercent,
                                             timeStretch, grainShape, octaveSpread, octaveProbability, midiNote);
                            }
                        }
                    }
                    samplesToNextGrain += baseSamplesPerGrain;
                }

                if (shouldGenerateGrains)
                    samplesToNextGrain -= 1.0;
            }

            // Mix all active grains with per-note envelopes
            // Use fixed-size array to avoid heap allocations on audio thread
            // Clear note outputs (MIDI notes are 0-127, -1 for no note)
            for (int i = 0; i < 128; ++i)
            {
                perNoteOutputs[i].left = 0.0f;
                perNoteOutputs[i].right = 0.0f;
                perNoteOutputs[i].hasOutput = false;
            }

            // Accumulate grain outputs per MIDI note
            for (auto& voice : grainVoices)
            {
                if (voice.activeVoice())
                {
                    auto [left, right] = voice.getNextSample();
                    int note = voice.getMidiNote();

                    if (note >= 0 && note < 128)
                    {
                        perNoteOutputs[note].left += left;
                        perNoteOutputs[note].right += right;
                        perNoteOutputs[note].hasOutput = true;
                    }
                }
            }

            // Apply per-note envelopes and sum
            float leftSample = 0.0f;
            float rightSample = 0.0f;

            for (int note = 0; note < 128; ++note)
            {
                if (perNoteOutputs[note].hasOutput)
                {
                    float left = perNoteOutputs[note].left;
                    float right = perNoteOutputs[note].right;

                    // Get envelope value for this note
                    auto it = noteEnvelopes.find(note);
                    if (it != noteEnvelopes.end())
                    {
                        float envValue = it->second.getNextValue();
                        left *= envValue;
                        right *= envValue;
                    }

                    leftSample += left;
                    rightSample += right;
                }
            }

            // Write to output buffer
            if (outputBuffer.getNumChannels() >= 2)
            {
                outputBuffer.addSample(0, sample, leftSample);
                outputBuffer.addSample(1, sample, rightSample);
            }
            else if (outputBuffer.getNumChannels() == 1)
            {
                // Mono: average left and right
                outputBuffer.addSample(0, sample, (leftSample + rightSample) * 0.5f);
            }
        }

        // Clean up finished envelopes (envelope handles tail fade internally now)
        for (auto it = noteEnvelopes.begin(); it != noteEnvelopes.end(); )
        {
            if (!it->second.isActive())
            {
                int noteToRemove = it->first;
                it = noteEnvelopes.erase(it);

                // Remove note from activeNotes now that envelope is completely done
                activeNotes.erase(noteToRemove);

                // Update current MIDI note if we still have active notes
                if (!activeNotes.empty())
                    currentMidiNote = *activeNotes.begin();
            }
            else
            {
                ++it;
            }
        }
    }

private:
    //==============================================================================
    /** Start a new grain with the given parameters (optimized - reuse distributions) */
    void startNewGrain(float position01, float sprayPercent, float grainSizeMs,
                       float pitchSemitones, float pitchSpreadSemitones,
                       float panPosition, float panSpreadPercent, float reversePercent,
                       float timeStretch, int grainShape,
                       float octaveSpread, float octaveProbability, int midiNote = -1)
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
                // Random octave shift: -octaveSpread to +octaveSpread octaves
                float octaveShift = (octaveDistribution(randomEngine) / 100.0f * 2.0f - 1.0f) * octaveSpread;
                // Round to nearest integer octave
                int octaves = static_cast<int>(std::round(octaveShift));
                // Convert to semitones (1 octave = 12 semitones)
                randomizedPitch += octaves * 12.0f;
            }
        }

        // Apply pan randomization
        const float panSpread01 = panSpreadPercent * 0.01f;
        const float randomizedPan = juce::jlimit(-1.0f, 1.0f,
            panPosition + panSpread01 * panDistribution(randomEngine));

        // Determine if this grain should be reversed
        const bool shouldReverse = reverseDistribution(randomEngine) < reversePercent;

        // Start the grain with all parameters including time stretch and shape
        voice->start(*sourceBuffer, randomizedPosition, grainSizeMs, randomizedPitch,
                     randomizedPan, shouldReverse, currentSampleRate, timeStretch, grainShape, midiNote);
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
    // Per-note output buffer struct (avoids heap allocations)
    struct NoteOutput
    {
        float left = 0.0f;
        float right = 0.0f;
        bool hasOutput = false;
    };

    // State
    std::vector<GrainVoice> grainVoices;
    const juce::AudioBuffer<float>* sourceBuffer = nullptr;
    std::mt19937 randomEngine;

    // Fixed-size per-note output buffer (128 MIDI notes)
    std::array<NoteOutput, 128> perNoteOutputs;

    // Pre-allocated random distributions (avoid construction overhead)
    std::uniform_real_distribution<float> sprayDistribution;
    std::uniform_real_distribution<float> pitchDistribution;
    std::uniform_real_distribution<float> panDistribution;
    std::uniform_real_distribution<float> reverseDistribution;
    std::uniform_real_distribution<float> octaveDistribution;

    double currentSampleRate = 44100.0;
    double samplesToNextGrain = 0.0;

    // Round-robin voice allocation
    int lastVoiceIndex = 0;

    // Freeze mode state
    bool wasFrozen = false;
    float frozenPosition = 0.5f;

    // MIDI note tracking
    std::set<int> activeNotes;
    int currentMidiNote = 60;  // Middle C as default

    // Per-note envelope system
    std::map<int, Envelope> noteEnvelopes;  // MIDI note -> envelope
    float envelopeAttack = 10.0f;
    float envelopeDecay = 300.0f;
    float envelopeSustain = 0.8f;  // 0-1 range
    float envelopeRelease = 500.0f;

    // Visualization state
    float lastActualPosition = 0.5f;  // The position parameter value (for UI display)
    float lastUsedPosition = 0.5f;    // The position actually used for grains (frozen or not)
    float lastSpray = 0.0f;
};
