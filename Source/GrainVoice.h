#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <random>

//==============================================================================
/**
 * GrainVoice - Represents a single grain of audio
 *
 * A grain is a short snippet of audio (typically 5-500ms) with an amplitude envelope.
 * Each grain has:
 * - A start position in the source sample
 * - A duration (grain size)
 * - A pitch shift (playback rate)
 * - A pan position
 * - An envelope that fades in and out (Hann window)
 */
class GrainVoice
{
public:
    GrainVoice() = default;

    //==============================================================================
    /** Start playing a new grain */
    void start(const juce::AudioBuffer<float>& sourceBuffer,
               float startPosition01,      // 0-1: position in sample
               float durationMs,            // Duration in milliseconds
               float pitchSemitones,        // Pitch shift in semitones
               float panPosition,           // -1 to +1: left to right
               double sampleRate)
    {
        isActive = true;
        currentPhase = 0.0;

        // Store source buffer reference
        source = &sourceBuffer;

        // Calculate grain parameters
        startSample = startPosition01 * (sourceBuffer.getNumSamples() - 1);
        grainLengthSamples = (durationMs / 1000.0) * sampleRate;

        // Convert semitones to playback rate
        // Formula: rate = 2^(semitones/12)
        playbackRate = std::pow(2.0, pitchSemitones / 12.0);

        // Store pan (-1 = left, 0 = center, +1 = right)
        pan = juce::jlimit(-1.0f, 1.0f, panPosition);
    }

    /** Check if grain is currently playing */
    bool activeVoice() const { return isActive; }

    /** Generate next sample and return stereo output */
    std::pair<float, float> getNextSample()
    {
        if (!isActive || source == nullptr)
            return {0.0f, 0.0f};

        // Calculate position in source buffer
        double readPosition = startSample + (currentPhase * playbackRate);

        // Check if we've reached the end of the grain
        if (currentPhase >= grainLengthSamples)
        {
            isActive = false;
            return {0.0f, 0.0f};
        }

        // Get mono sample from source (with linear interpolation)
        float sample = getSampleWithInterpolation(readPosition);

        // Apply Hann window envelope
        double progress = currentPhase / grainLengthSamples;
        float envelope = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * progress));

        float processedSample = sample * envelope;

        // Apply panning (constant power law)
        float leftGain = std::cos((pan + 1.0f) * juce::MathConstants<float>::pi / 4.0f);
        float rightGain = std::sin((pan + 1.0f) * juce::MathConstants<float>::pi / 4.0f);

        currentPhase += 1.0;

        return {processedSample * leftGain, processedSample * rightGain};
    }

private:
    //==============================================================================
    /** Get sample from source buffer with linear interpolation */
    float getSampleWithInterpolation(double position) const
    {
        if (source == nullptr || source->getNumSamples() == 0)
            return 0.0f;

        int index0 = static_cast<int>(position);
        int index1 = index0 + 1;

        // Wrap or clamp indices
        index0 = juce::jlimit(0, source->getNumSamples() - 1, index0);
        index1 = juce::jlimit(0, source->getNumSamples() - 1, index1);

        // Get samples from first channel (mono or left)
        float sample0 = source->getSample(0, index0);
        float sample1 = source->getSample(0, index1);

        // Linear interpolation
        float fraction = static_cast<float>(position - index0);
        return sample0 + fraction * (sample1 - sample0);
    }

    //==============================================================================
    // State
    bool isActive = false;
    const juce::AudioBuffer<float>* source = nullptr;

    // Grain parameters
    double startSample = 0.0;
    double grainLengthSamples = 0.0;
    double currentPhase = 0.0;
    double playbackRate = 1.0;
    float pan = 0.0f;
};
