#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

/**
 * ADSR Envelope Generator
 * Generates per-note envelope for amplitude control
 */
class Envelope
{
public:
    enum State
    {
        Idle,
        Attack,
        Decay,
        Sustain,
        Release,
        Tail  // Extended fade-out to let last grains finish smoothly
    };

    Envelope() = default;

    void setSampleRate(double sampleRate)
    {
        this->sampleRate = sampleRate;
    }

    void setAttack(float timeMs)
    {
        attackTime = juce::jmax(1.0f, timeMs);
    }

    void setDecay(float timeMs)
    {
        decayTime = juce::jmax(1.0f, timeMs);
    }

    void setSustain(float level)
    {
        sustainLevel = juce::jlimit(0.0f, 1.0f, level);
    }

    void setRelease(float timeMs)
    {
        releaseTime = juce::jmax(1.0f, timeMs);
    }

    void setTailTime(float timeMs)
    {
        tailTime = juce::jmax(50.0f, timeMs);  // At least 50ms tail
    }

    void noteOn()
    {
        state = Attack;
        currentLevel = 0.0f;
    }

    void noteOff()
    {
        if (state != Idle)
        {
            state = Release;
            releaseStartLevel = currentLevel;  // Start release from current level
        }
    }

    void reset()
    {
        state = Idle;
        currentLevel = 0.0f;
    }

    bool isActive() const
    {
        return state != Idle;
    }

    State getState() const
    {
        return state;
    }

    float getCurrentLevel() const
    {
        return currentLevel;
    }

    float getNextValue()
    {
        switch (state)
        {
            case Idle:
                currentLevel = 0.0f;
                break;

            case Attack:
            {
                // Linear ramp from 0 to 1
                float attackRate = 1.0f / (attackTime * 0.001f * sampleRate);
                currentLevel += attackRate;

                if (currentLevel >= 1.0f)
                {
                    currentLevel = 1.0f;
                    state = Decay;
                }
                break;
            }

            case Decay:
            {
                // Linear ramp from 1 to sustainLevel
                float decayRate = (1.0f - sustainLevel) / (decayTime * 0.001f * sampleRate);
                currentLevel -= decayRate;

                if (currentLevel <= sustainLevel)
                {
                    currentLevel = sustainLevel;
                    state = Sustain;
                }
                break;
            }

            case Sustain:
                currentLevel = sustainLevel;
                break;

            case Release:
            {
                // Linear ramp from current level toward 0
                float releaseRate = releaseStartLevel / (releaseTime * 0.001f * sampleRate);
                currentLevel -= releaseRate;

                if (currentLevel <= 0.01f)
                {
                    // Enter tail state instead of immediate silence
                    // This lets final grains fade out smoothly
                    currentLevel = 0.01f;
                    state = Tail;
                    tailStartLevel = 0.01f;
                }
                break;
            }

            case Tail:
            {
                // Quick fade from 0.01 to 0 over tailTime to let last grains finish
                // This prevents abrupt cutoff clicks
                // tailTime is set dynamically based on current grain size
                float tailRate = tailStartLevel / (tailTime * 0.001f * sampleRate);
                currentLevel -= tailRate;

                if (currentLevel <= 0.0f)
                {
                    currentLevel = 0.0f;
                    state = Idle;
                }
                break;
            }
        }

        return currentLevel;
    }

    // Process a block of samples and fill buffer with envelope values
    void processBlock(juce::AudioBuffer<float>& buffer)
    {
        int numSamples = buffer.getNumSamples();

        for (int i = 0; i < numSamples; ++i)
        {
            float envValue = getNextValue();

            // Apply to all channels
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                buffer.getWritePointer(ch)[i] *= envValue;
            }
        }
    }

private:
    double sampleRate = 44100.0;

    // ADSR parameters (milliseconds)
    float attackTime = 10.0f;
    float decayTime = 100.0f;
    float sustainLevel = 0.7f;
    float releaseTime = 500.0f;
    float tailTime = 100.0f;  // Dynamic tail time based on grain size

    // State
    State state = Idle;
    float currentLevel = 0.0f;
    float releaseStartLevel = 0.0f;  // Level when release was triggered
    float tailStartLevel = 0.0f;     // Level when tail was triggered
};
