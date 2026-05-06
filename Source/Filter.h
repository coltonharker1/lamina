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

        // One-time allocation: hand the duplicator a properly-sized Coefficients
        // object now (off the audio thread). Subsequent updates write directly into
        // its 5-element coefficient array — no audio-thread heap activity.
        *lowPassFilter.state  = *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, 1000.0f, 0.707f);
        *highPassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 1000.0f, 0.707f);

        lowPassFilter.reset();
        highPassFilter.reset();

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

    // RBJ biquad cookbook math written directly into the duplicator's pre-allocated
    // Coefficients<float>::coefficients array. This avoids the per-update heap
    // alloc/dealloc pair that Coefficients::makeLowPass/makeHighPass produces, which
    // is the standard JUCE real-time hazard when filter parameters are updated each
    // block.
    static void writeBiquadInPlace(juce::Array<float>& dest,
                                   double b0, double b1, double b2,
                                   double a0, double a1, double a2) noexcept
    {
        const float invA0 = static_cast<float>(1.0 / a0);
        dest.getReference(0) = static_cast<float>(b0) * invA0;
        dest.getReference(1) = static_cast<float>(b1) * invA0;
        dest.getReference(2) = static_cast<float>(b2) * invA0;
        dest.getReference(3) = static_cast<float>(a1) * invA0;
        dest.getReference(4) = static_cast<float>(a2) * invA0;
    }

    void updateLowPassCoefficients()
    {
        const double w     = 2.0 * juce::MathConstants<double>::pi * lowPassCutoff / currentSampleRate;
        const double cosW  = std::cos(w);
        const double alpha = std::sin(w) / (2.0 * lowPassResonance);

        writeBiquadInPlace(lowPassFilter.state->coefficients,
                           (1.0 - cosW) * 0.5,  // b0
                           (1.0 - cosW),         // b1
                           (1.0 - cosW) * 0.5,  // b2
                           1.0 + alpha,          // a0
                           -2.0 * cosW,          // a1
                           1.0 - alpha);         // a2
    }

    void updateHighPassCoefficients()
    {
        const double w     = 2.0 * juce::MathConstants<double>::pi * highPassCutoff / currentSampleRate;
        const double cosW  = std::cos(w);
        const double alpha = std::sin(w) / (2.0 * highPassResonance);

        writeBiquadInPlace(highPassFilter.state->coefficients,
                            (1.0 + cosW) * 0.5,  // b0
                           -(1.0 + cosW),         // b1
                            (1.0 + cosW) * 0.5,  // b2
                           1.0 + alpha,           // a0
                           -2.0 * cosW,           // a1
                           1.0 - alpha);          // a2
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
