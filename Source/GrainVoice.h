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
               bool shouldReverse,          // Play grain in reverse
               double sampleRate,
               float timeStretchParam,      // 0.25-4: time stretch multiplier (affects density, not pitch)
               int grainShapeParam,         // 0-4: envelope shape
               int midiNoteNumber = -1)     // Optional: MIDI note for polyphony
    {
        active = true;
        currentPhase = 0.0;
        midiNote = midiNoteNumber;
        envelopeShape = grainShapeParam;
        timeStretch = timeStretchParam;

        // Store source buffer reference
        source = &sourceBuffer;

        // Calculate grain parameters
        startSample = startPosition01 * (sourceBuffer.getNumSamples() - 1);

        // In granular synthesis, "time stretch" affects grain duration and overlap
        // Longer grains with more overlap = time stretched effect
        // This is different from Rubber Band style time stretching
        grainLengthSamples = (durationMs / 1000.0) * sampleRate * timeStretchParam;
        invGrainLength = 1.0 / grainLengthSamples; // Cache for faster division

        // Convert semitones to pitch ratio (2^(semitones/12))
        // This is simple pitch shifting via playback rate
        float pitchRatio = std::pow(2.0f, pitchSemitones / 12.0f);

        // Use cubic interpolation playback rate for smoother pitch shifting
        playbackRate = pitchRatio;

        // Reverse playback flips the direction
        if (shouldReverse)
            playbackRate = -playbackRate;

        // Pre-calculate pan gains (constant power law) to avoid trig per sample
        // This is a significant optimization - trig functions are expensive
        panPosition = juce::jlimit(-1.0f, 1.0f, panPosition);
        float panAngle = (panPosition + 1.0f) * juce::MathConstants<float>::pi * 0.25f;
        leftGain = std::cos(panAngle);
        rightGain = std::sin(panAngle);
    }

    /** Check if grain is currently playing */
    bool activeVoice() const { return active; }
    bool isActive() const { return active; }  // Alias for consistency

    /** Get current position in sample (0-1) for visualization */
    float getCurrentPosition() const
    {
        if (source == nullptr || source->getNumSamples() == 0)
            return 0.0f;
        return static_cast<float>((startSample + currentPhase * playbackRate) / source->getNumSamples());
    }

    /** Get grain progress through envelope (0-1) for visualization */
    float getProgress() const
    {
        if (grainLengthSamples <= 0.0)
            return 0.0f;
        return static_cast<float>(juce::jlimit(0.0, 1.0, currentPhase / grainLengthSamples));
    }

    /** Get pan position (-1 to +1) for visualization */
    float getPan() const
    {
        // Reconstruct pan from gains (approximate, for visualization only)
        return (std::atan2(rightGain, leftGain) * 4.0f / juce::MathConstants<float>::pi) - 1.0f;
    }

    /** Get MIDI note number for this grain */
    int getMidiNote() const
    {
        return midiNote;
    }

    /** Get grain size for visualization */
    float getGrainSize() const
    {
        return static_cast<float>(grainLengthSamples);
    }

    /** Generate next sample and return stereo output */
    inline std::pair<float, float> getNextSample()
    {
        if (!active || source == nullptr)
            return {0.0f, 0.0f};

        // Calculate position in source buffer
        const double readPosition = startSample + (currentPhase * playbackRate);

        // Check if we've reached the end of the grain
        if (currentPhase >= grainLengthSamples)
        {
            active = false;
            return {0.0f, 0.0f};
        }

        // Get mono sample from source with CUBIC interpolation (smoother than linear)
        const float sample = getSampleWithCubicInterpolation(readPosition);

        // Apply envelope based on selected shape (using cached 1/grainLength for faster division)
        const float progress = static_cast<float>(currentPhase * invGrainLength);
        const float envelope = calculateEnvelope(progress);

        const float processedSample = sample * envelope;

        // Apply pre-calculated pan gains (no trig functions in hot path!)
        currentPhase += 1.0;

        return {processedSample * leftGain, processedSample * rightGain};
    }

private:
    //==============================================================================
    /** Calculate envelope value based on shape type (optimized, branchless where possible) */
    inline float calculateEnvelope(float progress) const
    {
        // Clamp progress to 0-1
        progress = juce::jlimit(0.0f, 1.0f, progress);

        switch (envelopeShape)
        {
            case 0: // Hann window (default, smooth)
            {
                const float angle = juce::MathConstants<float>::pi * progress;
                return 0.5f * (1.0f - std::cos(2.0f * angle));
            }

            case 1: // Triangle (linear attack/release) - branchless
            {
                const float centered = 2.0f * progress - 1.0f;
                return 1.0f - std::abs(centered);
            }

            case 2: // Trapezoid (sustain in middle)
            {
                if (progress < 0.25f)
                    return progress * 4.0f;  // Attack
                else if (progress < 0.75f)
                    return 1.0f;  // Sustain
                else
                    return (1.0f - progress) * 4.0f;  // Release
            }

            case 3: // Exponential (natural decay feel)
            {
                // Exp attack, exp release
                if (progress < 0.5f)
                    return 1.0f - std::exp(-10.0f * progress);
                else
                    return std::exp(-10.0f * (progress - 0.5f));
            }

            case 4: // Gaussian (very smooth, bell curve)
            {
                const float x = (progress - 0.5f) * 6.0f;  // -3 to +3
                return std::exp(-x * x * 0.5f);
            }

            default:
            {
                const float angle = juce::MathConstants<float>::pi * progress;
                return 0.5f * (1.0f - std::cos(2.0f * angle));
            }
        }
    }

    /** Get sample from source buffer with optimized cubic interpolation (Hermite - fewer ops than Catmull-Rom) */
    inline float getSampleWithCubicInterpolation(double position) const
    {
        if (source == nullptr || source->getNumSamples() == 0)
            return 0.0f;

        const int maxIndex = source->getNumSamples() - 1;
        const int index1 = static_cast<int>(position);

        // Early bounds check for common case
        if (index1 < 1 || index1 >= maxIndex - 1)
        {
            // Near edges - use linear interpolation (faster, good enough at boundaries)
            const int i0 = juce::jlimit(0, maxIndex, index1);
            const int i1 = juce::jlimit(0, maxIndex, index1 + 1);
            const float y0 = source->getSample(0, i0);
            const float y1 = source->getSample(0, i1);
            const float frac = static_cast<float>(position - index1);
            return y0 + frac * (y1 - y0);
        }

        // Middle of buffer - use cubic Hermite (4-point, 3rd order)
        const float* data = source->getReadPointer(0);
        const float y0 = data[index1 - 1];
        const float y1 = data[index1];
        const float y2 = data[index1 + 1];
        const float y3 = data[index1 + 2];

        // Fractional position
        const float t = static_cast<float>(position - index1);

        // Optimized cubic Hermite interpolation (fewer multiplications than Catmull-Rom)
        // Pre-computed constants for better compiler optimization
        const float c0 = y1;
        const float c1 = 0.5f * (y2 - y0);
        const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        const float c3 = 1.5f * (y1 - y2) + 0.5f * (y3 - y0);

        // Horner's method for polynomial evaluation (more efficient)
        return c0 + t * (c1 + t * (c2 + t * c3));
    }

    //==============================================================================
    // State
    bool active = false;
    const juce::AudioBuffer<float>* source = nullptr;

    // Grain parameters
    double startSample = 0.0;
    double grainLengthSamples = 0.0;
    double invGrainLength = 1.0;  // Cached 1/grainLength for faster division
    double currentPhase = 0.0;
    double playbackRate = 1.0;
    int midiNote = -1;  // -1 = no MIDI note

    // Pre-calculated pan gains (avoid trig in hot path)
    float leftGain = 0.707f;   // Default center position
    float rightGain = 0.707f;

    // Phase 3 parameters
    int envelopeShape = 0;    // 0-4: envelope window type
    float timeStretch = 1.0f;  // 0.25-4: time stretch multiplier (affects grain duration/overlap)
};
