#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "GrainVoice.h"
#include <vector>
#include <random>
#include <set>

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
        // Initialize grain voice pool (max 64 simultaneous grains)
        grainVoices.resize(64);

        // Initialize random number generator
        randomEngine.seed(std::random_device{}());
    }

    //==============================================================================
    /** Prepare for playback */
    void prepare(double sampleRate, int samplesPerBlock)
    {
        currentSampleRate = sampleRate;
        samplesToNextGrain = 0.0;
    }

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
    }

    /** Note off - stop playing grains */
    void noteOff(int midiNote)
    {
        activeNotes.erase(midiNote);

        // Update current note if we still have active notes
        if (!activeNotes.empty())
            currentMidiNote = *activeNotes.begin();  // Use lowest note
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
                 float reversePercent)       // 0-100: reverse probability
    {
        if (!hasSource())
            return;

        const int numSamples = outputBuffer.getNumSamples();

        // Calculate how many samples between grain starts
        double samplesPerGrain = currentSampleRate / densityGrainsPerSec;

        // Store freeze position on first freeze activation
        if (freezeMode && !wasFrozen)
        {
            frozenPosition = position01;
            wasFrozen = true;
        }
        else if (!freezeMode)
        {
            wasFrozen = false;
        }

        // Use frozen position if freeze is active
        float actualPosition = freezeMode ? frozenPosition : position01;

        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Only generate new grains if notes are being held
            if (hasActiveNotes())
            {
                // Check if it's time to start a new grain
                if (samplesToNextGrain <= 0.0)
                {
                    // Add MIDI note pitch offset to the base pitch parameter
                    float totalPitch = pitchSemitones + getMidiPitchOffset();

                    startNewGrain(actualPosition, sprayPercent, grainSizeMs,
                                 totalPitch, pitchSpreadSemitones,
                                 panPosition, panSpreadPercent, reversePercent);
                    samplesToNextGrain += samplesPerGrain;
                }

                samplesToNextGrain -= 1.0;
            }

            // Mix all active grains
            float leftSample = 0.0f;
            float rightSample = 0.0f;

            for (auto& voice : grainVoices)
            {
                if (voice.activeVoice())
                {
                    auto [left, right] = voice.getNextSample();
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
    }

private:
    //==============================================================================
    /** Start a new grain with the given parameters */
    void startNewGrain(float position01, float sprayPercent, float grainSizeMs,
                       float pitchSemitones, float pitchSpreadSemitones,
                       float panPosition, float panSpreadPercent, float reversePercent)
    {
        // Find an inactive voice
        GrainVoice* voice = findFreeVoice();
        if (voice == nullptr)
            return;  // No free voices available

        // Apply spray (randomization) to position
        float spray01 = sprayPercent / 100.0f;
        std::uniform_real_distribution<float> sprayDist(-spray01, spray01);
        float randomizedPosition = position01 + sprayDist(randomEngine);
        randomizedPosition = juce::jlimit(0.0f, 1.0f, randomizedPosition);

        // Apply pitch randomization
        std::uniform_real_distribution<float> pitchDist(-pitchSpreadSemitones, pitchSpreadSemitones);
        float randomizedPitch = pitchSemitones + pitchDist(randomEngine);

        // Apply pan randomization
        float panSpread01 = panSpreadPercent / 100.0f;
        std::uniform_real_distribution<float> panDist(-panSpread01, panSpread01);
        float randomizedPan = panPosition + panDist(randomEngine);
        randomizedPan = juce::jlimit(-1.0f, 1.0f, randomizedPan);

        // Determine if this grain should be reversed
        std::uniform_real_distribution<float> reverseDist(0.0f, 100.0f);
        bool shouldReverse = reverseDist(randomEngine) < reversePercent;

        // Start the grain
        voice->start(*sourceBuffer, randomizedPosition, grainSizeMs, randomizedPitch,
                     randomizedPan, shouldReverse, currentSampleRate);
    }

    /** Find an inactive grain voice, or steal the oldest one */
    GrainVoice* findFreeVoice()
    {
        // First try to find an inactive voice
        for (auto& voice : grainVoices)
        {
            if (!voice.activeVoice())
                return &voice;
        }

        // No free voices, use the first one (simple voice stealing)
        return &grainVoices[0];
    }

    //==============================================================================
    // State
    std::vector<GrainVoice> grainVoices;
    const juce::AudioBuffer<float>* sourceBuffer = nullptr;
    std::mt19937 randomEngine;

    double currentSampleRate = 44100.0;
    double samplesToNextGrain = 0.0;

    // Freeze mode state
    bool wasFrozen = false;
    float frozenPosition = 0.5f;

    // MIDI note tracking
    std::set<int> activeNotes;
    int currentMidiNote = 60;  // Middle C as default
};
