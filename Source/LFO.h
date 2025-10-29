#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <random>

//==============================================================================
/**
 * LFO - Low Frequency Oscillator for parameter modulation
 *
 * Features:
 * - 6 waveforms: Sine, Triangle, Saw Up, Saw Down, Square, Random (S&H)
 * - Rate: 0.01 Hz - 20 Hz (or tempo-synced)
 * - Depth: 0-100% modulation amount
 * - Phase offset: 0-360 degrees
 * - Efficient: Updates at control rate (every 32 samples)
 */
class LFO
{
public:
    enum Waveform
    {
        Sine = 0,
        Triangle,
        SawUp,
        SawDown,
        Square,
        Random  // Sample & Hold
    };

    LFO()
    {
        randomEngine.seed(std::random_device{}());
        randomDistribution = std::uniform_real_distribution<float>(-1.0f, 1.0f);
    }

    //==============================================================================
    /** Prepare for playback */
    void prepare(double sampleRate)
    {
        currentSampleRate = sampleRate;
        phase = 0.0;
        currentValue = 0.0f;
        smoothedValue = 0.0f;
        controlRateCounter = 0;
    }

    /** Set parameters */
    void setRate(float hz) { rateHz = juce::jlimit(0.01f, 20.0f, hz); }
    void setDepth(float depth01) { depthAmount = juce::jlimit(0.0f, 1.0f, depth01); }
    void setWaveform(Waveform wave) { waveform = wave; }
    void setPhaseOffset(float degrees) { phaseOffset = degrees / 360.0f; }
    void setTempoSync(bool sync) { tempoSynced = sync; }
    void setTempoRate(float bpm, float noteValue)
    {
        tempoBPM = bpm;
        tempoNoteValue = noteValue;  // e.g., 1.0 = quarter note, 0.5 = eighth note
    }

    /** Get current LFO value (-depth to +depth) */
    float getValue() const { return smoothedValue * depthAmount; }

    /** Process a block of audio (updates LFO at control rate) */
    void process(int numSamples)
    {
        // Update at control rate (every 32 samples) for efficiency
        controlRateCounter += numSamples;

        if (controlRateCounter >= CONTROL_RATE_SAMPLES)
        {
            controlRateCounter = 0;
            updateLFO();
        }

        // Smooth the value to prevent zipper noise
        smoothedValue += SMOOTHING_ALPHA * (currentValue - smoothedValue);
    }

    /** Reset phase to beginning */
    void reset()
    {
        phase = phaseOffset;
        if (waveform == Random)
            currentValue = randomDistribution(randomEngine);
    }

private:
    //==============================================================================
    /** Update LFO value (called at control rate) */
    void updateLFO()
    {
        // Calculate effective rate (Hz or tempo-synced)
        float effectiveRate = rateHz;

        if (tempoSynced && tempoBPM > 0.0f)
        {
            // Convert tempo to Hz: (BPM / 60) * note value
            // Quarter note at 120 BPM = 2 Hz
            effectiveRate = (tempoBPM / 60.0f) * tempoNoteValue;
        }

        // Calculate phase increment
        const double phaseIncrement = effectiveRate / currentSampleRate * CONTROL_RATE_SAMPLES;
        phase += phaseIncrement;

        // Wrap phase (0-1)
        if (phase >= 1.0)
        {
            phase -= 1.0;

            // Update random value on cycle boundary (sample & hold)
            if (waveform == Random)
                currentValue = randomDistribution(randomEngine);
        }

        // Calculate waveform value based on phase
        float phaseWithOffset = std::fmod(phase + phaseOffset, 1.0);
        currentValue = calculateWaveform(phaseWithOffset);
    }

    /** Calculate waveform value at given phase (0-1) */
    inline float calculateWaveform(float p) const
    {
        switch (waveform)
        {
            case Sine:
                return std::sin(p * juce::MathConstants<float>::twoPi);

            case Triangle:
            {
                // Triangle: ramps up 0->1 (first half), down 1->0 (second half)
                if (p < 0.5f)
                    return -1.0f + 4.0f * p;  // -1 to +1
                else
                    return 3.0f - 4.0f * p;   // +1 to -1
            }

            case SawUp:
                // Saw up: linear ramp from -1 to +1
                return -1.0f + 2.0f * p;

            case SawDown:
                // Saw down: linear ramp from +1 to -1
                return 1.0f - 2.0f * p;

            case Square:
                // Square: -1 for first half, +1 for second half
                return (p < 0.5f) ? -1.0f : 1.0f;

            case Random:
                // Random: sample & hold (value updated on phase wrap)
                return currentValue;

            default:
                return 0.0f;
        }
    }

    //==============================================================================
    // Constants
    static constexpr int CONTROL_RATE_SAMPLES = 32;  // Update every 32 samples
    static constexpr float SMOOTHING_ALPHA = 0.05f;   // Smoothing coefficient

    // State
    double currentSampleRate = 44100.0;
    double phase = 0.0;               // Current phase (0-1)
    float currentValue = 0.0f;        // Current waveform value (-1 to +1)
    float smoothedValue = 0.0f;       // Smoothed output value
    int controlRateCounter = 0;       // Counts samples until next LFO update

    // Parameters
    float rateHz = 1.0f;              // LFO rate in Hz
    float depthAmount = 0.5f;         // Modulation depth (0-1)
    float phaseOffset = 0.0f;         // Phase offset (0-1)
    Waveform waveform = Sine;         // Current waveform type

    // Tempo sync
    bool tempoSynced = false;
    float tempoBPM = 120.0f;
    float tempoNoteValue = 1.0f;      // 1.0 = quarter, 0.5 = eighth, 2.0 = half

    // Random generation
    std::mt19937 randomEngine;
    std::uniform_real_distribution<float> randomDistribution;
};
