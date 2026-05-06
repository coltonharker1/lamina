#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "LookupTables.h"
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
 * - Per-grain filter (one-pole lowpass, stereo)
 * - Stereo spread (Haas effect for spatial width)
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
               int grainShapeParam,         // 0-4: envelope shape
               int midiNoteNumber = -1,     // Optional: MIDI note for polyphony
               float filterCutoffHz = 20000.0f,  // Per-grain filter cutoff
               float stereoSpreadSamples = 0.0f)  // Stereo delay spread in samples
    {
        active = true;
        currentPhase = 0.0;
        midiNote = midiNoteNumber;
        envelopeShape = grainShapeParam;

        // Store source buffer reference
        source = &sourceBuffer;

        // Calculate grain parameters
        // Grain duration based on Size parameter only
        grainLengthSamples = (durationMs / 1000.0) * sampleRate;
        invGrainLength = 1.0 / grainLengthSamples; // Cache for faster division

        // Convert semitones to pitch ratio using lookup table (replaces expensive std::pow)
        float pitchRatio = LookupTables::getInstance().getPitchRatio(pitchSemitones);
        playbackRate = pitchRatio;

        // Reverse playback flips the direction
        if (shouldReverse)
            playbackRate = -playbackRate;

        // CRITICAL: Ensure grain doesn't read past buffer end
        // Calculate maximum read extent considering pitch shift
        const double maxGrainExtent = grainLengthSamples * std::abs(playbackRate);
        const double bufferSize = static_cast<double>(sourceBuffer.getNumSamples());

        // Clamp start position to ensure we don't read past buffer bounds
        // Leave safety margin for cubic interpolation (needs +2 samples)
        const double maxStartPosition = bufferSize - maxGrainExtent - 4.0;
        const double requestedStart = startPosition01 * (bufferSize - 1.0);
        startSample = juce::jlimit(0.0, std::max(0.0, maxStartPosition), requestedStart);

        // Pre-calculate pan gains (constant power law) using lookup tables
        // Maps pan -1..+1 to 0..1 for table lookup
        panPosition = juce::jlimit(-1.0f, 1.0f, panPosition);
        float panNormalized = (panPosition + 1.0f) * 0.5f;  // 0 to 1
        const auto& tables = LookupTables::getInstance();
        leftGain = tables.getCos(panNormalized);
        rightGain = tables.getSin(panNormalized);

        // Initialize per-grain filter (one-pole lowpass)
        // Use direct calculation for accuracy at any sample rate
        filterCutoffHz = juce::jlimit(20.0f, 20000.0f, filterCutoffHz);
        float normalizedCutoff = filterCutoffHz / static_cast<float>(sampleRate);
        filterCoeff = std::exp(-2.0f * juce::MathConstants<float>::pi * normalizedCutoff);
        filterStateL = 0.0f;  // Reset filter states for new grain
        filterStateR = 0.0f;

        // Store stereo spread for spatial width (Haas effect)
        stereoSpread = stereoSpreadSamples;
    }

    /** Check if grain is currently playing */
    bool activeVoice() const { return active; }
    bool isActive() const { return active; }  // Alias for consistency

    /** Deactivate the grain (for safe buffer swapping) */
    void deactivate() { active = false; }

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

        // Check if we've reached the end of the grain
        if (currentPhase >= grainLengthSamples)
        {
            active = false;
            return {0.0f, 0.0f};
        }

        const int bufferSize = source->getNumSamples();

        // Calculate positions for left and right channels with stereo spread
        // Stereo spread creates Haas effect for width: small delays (<30ms) widen the image
        const double baseReadPosition = startSample + (currentPhase * playbackRate);
        const double leftReadPosition = baseReadPosition - stereoSpread;
        const double rightReadPosition = baseReadPosition + stereoSpread;

        // CRITICAL SAFETY CHECK: Detect out-of-bounds reads
        if (baseReadPosition < 0.0 || baseReadPosition >= bufferSize - 4 ||
            leftReadPosition < 0.0 || leftReadPosition >= bufferSize - 4 ||
            rightReadPosition < 0.0 || rightReadPosition >= bufferSize - 4)
        {
            active = false;  // Kill this grain to prevent crash
            return {0.0f, 0.0f};
        }

        // Get samples for left and right channels with different read positions
        float leftSample = getSampleWithCubicInterpolation(leftReadPosition);
        float rightSample = getSampleWithCubicInterpolation(rightReadPosition);

        // Apply per-grain lowpass filter independently to each channel (preserves stereo).
        // filterCoeff = exp(-2π·fc/fs): small at high cutoff (near-transparent), large
        // at low cutoff (heavy filtering). Engage whenever the filter is doing real
        // work; only bypass when it would be effectively a no-op.
        if (filterCoeff > 0.01f)
        {
            filterStateL = (1.0f - filterCoeff) * leftSample + filterCoeff * filterStateL;
            filterStateR = (1.0f - filterCoeff) * rightSample + filterCoeff * filterStateR;
            leftSample = filterStateL;
            rightSample = filterStateR;
        }

        // Apply envelope based on selected shape (using cached 1/grainLength for faster division)
        const float progress = static_cast<float>(currentPhase * invGrainLength);
        const float envelope = calculateEnvelope(progress);

        const float leftProcessed = leftSample * envelope;
        const float rightProcessed = rightSample * envelope;

        // Apply pre-calculated pan gains (no trig functions in hot path!)
        currentPhase += 1.0;

        return {leftProcessed * leftGain, rightProcessed * rightGain};
    }

private:
    //==============================================================================
    /** Calculate envelope value based on shape type using lookup tables */
    inline float calculateEnvelope(float progress) const
    {
        // Clamp progress to 0-1
        progress = juce::jlimit(0.0f, 1.0f, progress);
        const auto& tables = LookupTables::getInstance();

        switch (envelopeShape)
        {
            case 0: // Hann window - use lookup table
                return tables.getHann(progress);

            case 1: // Triangle (linear attack/release) - branchless, no expensive math
            {
                const float centered = 2.0f * progress - 1.0f;
                return 1.0f - std::abs(centered);
            }

            case 2: // Trapezoid (sustain in middle) - simple branches, no expensive math
            {
                if (progress < 0.25f)
                    return progress * 4.0f;
                else if (progress < 0.75f)
                    return 1.0f;
                else
                    return (1.0f - progress) * 4.0f;
            }

            case 3: // Exponential - use lookup tables
            {
                if (progress < 0.5f)
                    return tables.getExpAttack(progress);
                else
                    return tables.getExpDecay(progress);
            }

            case 4: // Gaussian - use lookup table
                return tables.getGaussian(progress);

            default:
                return tables.getHann(progress);
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

    // Grain envelope shape
    int envelopeShape = 0;    // 0-4: envelope window type

    // Per-grain filter state (one-pole lowpass, stereo)
    float filterCoeff = 0.0f;   // Filter coefficient (0 = no filtering, 1 = max smoothing)
    float filterStateL = 0.0f;  // Previous output sample (left)
    float filterStateR = 0.0f;  // Previous output sample (right)

    // Stereo spread (delay offset in samples for Haas effect)
    float stereoSpread = 0.0f;  // Positive = left delayed, negative = right delayed
};
