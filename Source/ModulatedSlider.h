#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/**
 * ModulatedSlider - A slider that shows a visual glow when being modulated by an LFO
 *
 * Features:
 * - Draws a pulsing glow ring around the slider when modulated
 * - Automatically fades in/out based on modulation state
 * - Uses Lamina's color scheme (cyan glow)
 */
class ModulatedSlider : public juce::Slider
{
public:
    ModulatedSlider() = default;

    ~ModulatedSlider() override = default;

    /** Set whether this slider is currently being modulated */
    void setModulated(bool isModulated)
    {
        if (modulated != isModulated)
        {
            modulated = isModulated;
            repaint();
        }
    }

    /** Check if this slider is being modulated */
    bool isModulated() const { return modulated; }

    void paint(juce::Graphics& g) override
    {
        // First, draw the standard slider
        juce::Slider::paint(g);

        // If modulated, draw a glow ring around the knob
        if (modulated && getSliderStyle() == juce::Slider::RotaryHorizontalVerticalDrag)
        {
            auto bounds = getLocalBounds().toFloat();

            // Find the knob area (excluding text box)
            auto textBoxHeight = getTextBoxHeight();
            auto knobBounds = bounds.withTrimmedBottom(textBoxHeight + 4);

            // Draw pulsing glow ring
            auto centerX = knobBounds.getCentreX();
            auto centerY = knobBounds.getCentreY();
            auto radius = juce::jmin(knobBounds.getWidth(), knobBounds.getHeight()) * 0.5f;

            // Calculate pulse (0.5 - 1.0 range for subtle pulse)
            float pulse = 0.75f + 0.25f * std::sin(pulsePhase);

            // Draw multiple glow rings with different alphas for soft glow effect
            for (int i = 3; i >= 0; --i)
            {
                float ringRadius = radius + (i * 2.5f);
                float alpha = (0.15f - (i * 0.03f)) * pulse;

                g.setColour(juce::Colour(0xff7dd3fc).withAlpha(alpha));  // Soft cyan glow
                g.drawEllipse(centerX - ringRadius, centerY - ringRadius,
                             ringRadius * 2.0f, ringRadius * 2.0f, 2.0f);
            }

            // Update pulse phase (will be incremented by timer in parent)
            // Phase wraps from 0 to 2π
        }
    }

    /** Update the pulse animation (call this from a timer) */
    void updatePulse(float deltaPhase = 0.1f)
    {
        if (modulated)
        {
            pulsePhase += deltaPhase;
            if (pulsePhase > juce::MathConstants<float>::twoPi)
                pulsePhase -= juce::MathConstants<float>::twoPi;

            repaint();
        }
    }

private:
    bool modulated = false;
    float pulsePhase = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulatedSlider)
};
