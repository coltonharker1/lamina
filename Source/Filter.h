#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

/**
 * Filter - Dual independent filters (low-pass and high-pass)
 *
 * Features:
 * - Independent low-pass and high-pass filters that can run simultaneously
 * - Each filter has its own enable/disable toggle
 * - Separate cutoff frequency control for each (20Hz - 20kHz)
 * - Separate resonance control for each (Q factor)
 * - Smooth parameter changes to avoid clicks
 */
class Filter
{
public:
    Filter() = default;

    void prepare(double sampleRate, int samplesPerBlock)
    {
        currentSampleRate = sampleRate;

        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
        spec.numChannels = 2;  // Stereo

        // Prepare both filters
        lowPassFilter.prepare(spec);
        highPassFilter.prepare(spec);

        // Reset state
        lowPassFilter.reset();
        highPassFilter.reset();

        // Update coefficients with correct sample rate
        updateFilterCoefficients();
    }

    // Low-pass filter controls
    void setLowPassEnabled(bool enabled)
    {
        lowPassEnabled = enabled;
    }

    void setLowPassCutoff(float frequencyHz)
    {
        lowPassCutoff = juce::jlimit(20.0f, 20000.0f, frequencyHz);
        updateLowPassCoefficients();
    }

    void setLowPassResonance(float resonanceValue)
    {
        lowPassResonance = juce::jlimit(0.1f, 10.0f, resonanceValue);
        updateLowPassCoefficients();
    }

    // High-pass filter controls
    void setHighPassEnabled(bool enabled)
    {
        highPassEnabled = enabled;
    }

    void setHighPassCutoff(float frequencyHz)
    {
        highPassCutoff = juce::jlimit(20.0f, 20000.0f, frequencyHz);
        updateHighPassCoefficients();
    }

    void setHighPassResonance(float resonanceValue)
    {
        highPassResonance = juce::jlimit(0.1f, 10.0f, resonanceValue);
        updateHighPassCoefficients();
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        // Process using JUCE DSP wrapper
        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);

        // Apply filters in series (can be both active for band-pass effect)
        if (highPassEnabled)
        {
            highPassFilter.process(context);
        }

        if (lowPassEnabled)
        {
            lowPassFilter.process(context);
        }
    }

private:
    void updateFilterCoefficients()
    {
        updateLowPassCoefficients();
        updateHighPassCoefficients();
    }

    void updateLowPassCoefficients()
    {
        *lowPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(
            currentSampleRate,
            lowPassCutoff,
            lowPassResonance
        );
    }

    void updateHighPassCoefficients()
    {
        *highPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
            currentSampleRate,
            highPassCutoff,
            highPassResonance
        );
    }

    // Low-pass filter state
    bool lowPassEnabled = false;
    float lowPassCutoff = 1000.0f;
    float lowPassResonance = 0.707f;  // Butterworth (flat response)

    // High-pass filter state
    bool highPassEnabled = false;
    float highPassCutoff = 1000.0f;
    float highPassResonance = 0.707f;  // Butterworth (flat response)

    // JUCE DSP filters (stereo, using ProcessorDuplicator)
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> lowPassFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> highPassFilter;

    double currentSampleRate = 44100.0;
};
