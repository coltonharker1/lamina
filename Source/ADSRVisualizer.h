#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * ADSRVisualizer - Visual representation of the ADSR envelope
 *
 * Displays the Attack, Decay, Sustain, Release stages as a curve
 * Updates in real-time as parameters change
 */
class ADSRVisualizer : public juce::Component,
                       public juce::Timer
{
public:
    ADSRVisualizer()
    {
        startTimerHz(30);  // Update at 30 fps
    }

    ~ADSRVisualizer() override
    {
        stopTimer();
    }

    // Set parameter pointers to track ADSR values
    void setAttackParameter(juce::AudioParameterFloat* param) { attackParam = param; }
    void setDecayParameter(juce::AudioParameterFloat* param) { decayParam = param; }
    void setSustainParameter(juce::AudioParameterFloat* param) { sustainParam = param; }
    void setReleaseParameter(juce::AudioParameterFloat* param) { releaseParam = param; }

    void paint(juce::Graphics& g) override
    {
        // Background
        g.fillAll(juce::Colour(0xff0f1620));

        // Border
        g.setColour(juce::Colour(0xff4a4a52));
        g.drawRect(getLocalBounds(), 1);

        // Get current ADSR values
        float attack = attackParam ? attackParam->get() : 10.0f;
        float decay = decayParam ? decayParam->get() : 300.0f;
        float sustain = sustainParam ? sustainParam->get() : 0.8f;  // 0-1 range
        float release = releaseParam ? releaseParam->get() : 500.0f;

        // Calculate normalized time values (for visual spacing)
        // Total time = attack + decay + (some sustain hold time) + release
        const float sustainHoldTime = 200.0f;  // Visual hold time for sustain
        float totalTime = attack + decay + sustainHoldTime + release;

        float attackNorm = attack / totalTime;
        float decayNorm = decay / totalTime;
        float sustainHoldNorm = sustainHoldTime / totalTime;
        float releaseNorm = release / totalTime;

        // Draw the envelope curve
        juce::Path envelopePath;

        const float padding = 10.0f;
        const float width = getWidth() - (padding * 2);
        const float height = getHeight() - (padding * 2);

        // Start point (0, 0)
        float x = padding;
        float y = padding + height;
        envelopePath.startNewSubPath(x, y);

        // Attack phase (0 -> 1)
        x = padding + (width * attackNorm);
        y = padding;
        envelopePath.lineTo(x, y);

        // Decay phase (1 -> sustain)
        x = padding + (width * (attackNorm + decayNorm));
        y = padding + (height * (1.0f - sustain));
        envelopePath.lineTo(x, y);

        // Sustain hold (flat line at sustain level)
        x = padding + (width * (attackNorm + decayNorm + sustainHoldNorm));
        envelopePath.lineTo(x, y);

        // Release phase (sustain -> 0)
        x = padding + width;
        y = padding + height;
        envelopePath.lineTo(x, y);

        // Draw the curve
        g.setColour(juce::Colour(0xffb4a5d8).withAlpha(0.8f));  // Soft lavender
        g.strokePath(envelopePath, juce::PathStrokeType(2.0f));

        // Draw phase labels
        g.setColour(juce::Colour(0xff9ba4d8).withAlpha(0.6f));
        g.setFont(juce::Font(juce::FontOptions(10.0f)));

        // Attack label
        float labelX = padding + (width * attackNorm * 0.5f);
        g.drawText("A", labelX - 10, getHeight() - 18, 20, 12, juce::Justification::centred);

        // Decay label
        labelX = padding + (width * (attackNorm + decayNorm * 0.5f));
        g.drawText("D", labelX - 10, getHeight() - 18, 20, 12, juce::Justification::centred);

        // Sustain label
        labelX = padding + (width * (attackNorm + decayNorm + sustainHoldNorm * 0.5f));
        g.drawText("S", labelX - 10, getHeight() - 18, 20, 12, juce::Justification::centred);

        // Release label
        labelX = padding + (width * (attackNorm + decayNorm + sustainHoldNorm + releaseNorm * 0.5f));
        g.drawText("R", labelX - 10, getHeight() - 18, 20, 12, juce::Justification::centred);

        // Draw value text
        g.setColour(juce::Colour(0xfff5f1e8).withAlpha(0.7f));
        g.setFont(juce::Font(juce::FontOptions(9.0f)));

        juce::String valuesText = juce::String(attack, 0) + "ms / " +
                                  juce::String(decay, 0) + "ms / " +
                                  juce::String(sustain * 100.0f, 0) + "% / " +
                                  juce::String(release, 0) + "ms";
        g.drawText(valuesText, getLocalBounds().withTrimmedTop(2), juce::Justification::centredTop);
    }

    void timerCallback() override
    {
        // Repaint to update visualization if parameters change
        repaint();
    }

private:
    juce::AudioParameterFloat* attackParam = nullptr;
    juce::AudioParameterFloat* decayParam = nullptr;
    juce::AudioParameterFloat* sustainParam = nullptr;
    juce::AudioParameterFloat* releaseParam = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ADSRVisualizer)
};
