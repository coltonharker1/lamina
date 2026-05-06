#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "GrainEngine.h"

//==============================================================================
/**
 * GrainVisualizer - Shows active grains and their positions in the sample
 * Similar to the web version visualization
 */
class GrainVisualizer : public juce::Component,
                        private juce::Timer
{
public:
    GrainVisualizer()
    {
        startTimerHz(60);  // 60 FPS for smooth grain animation
    }

    ~GrainVisualizer() override
    {
        stopTimer();
    }

    //==============================================================================
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();

        // Lamina sleepy theme - darker background
        g.fillAll(juce::Colour(0xff0f1620));

        // Subtle border
        g.setColour(juce::Colour(0xff4a4a52).withAlpha(0.3f));
        g.drawRect(bounds, 1);

        if (grainEngine == nullptr)
        {
            g.setColour(juce::Colour(0xff9ba4d8).withAlpha(0.5f));
            g.setFont(12.0f);
            g.drawText("Grain Activity", bounds, juce::Justification::centred);
            return;
        }

        // Draw grain window visualization
        drawGrainWindows(g, bounds);

        // Label
        g.setColour(juce::Colour(0xfff5f1e8).withAlpha(0.6f));
        g.setFont(10.0f);
        g.drawText("GRAIN ACTIVITY", bounds.removeFromTop(15), juce::Justification::centred);
    }

    //==============================================================================
    void setGrainEngine(GrainEngine* engine)
    {
        grainEngine = engine;
    }

private:
    //==============================================================================
    void timerCallback() override
    {
        repaint();
    }

    void drawGrainWindows(juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        if (grainEngine == nullptr)
            return;

        bounds = bounds.reduced(10, 20);

        // Get active voices from grain engine
        const auto& voices = grainEngine->getVoices();

        int activeCount = 0;
        for (const auto& voice : voices)
        {
            if (voice.isActive())
            {
                activeCount++;

                // Calculate grain position and progress
                float position = voice.getCurrentPosition();
                float progress = voice.getProgress();  // 0-1 through grain envelope

                // Position on x-axis
                int x = bounds.getX() + static_cast<int>(position * bounds.getWidth());

                // Size based on grain envelope (bigger in middle, smaller at start/end)
                float envelopeSize = 1.0f - std::abs(progress * 2.0f - 1.0f);
                int radius = static_cast<int>(3.0f + envelopeSize * 8.0f);

                // Y position - spread vertically for multiple grains
                int y = bounds.getY() + (bounds.getHeight() / 2) +
                        static_cast<int>(std::sin(position * 10.0f) * bounds.getHeight() * 0.3f);

                // Color based on progress - soft lavender to dusty rose gradient
                juce::Colour startColor(0xffb4a5d8);  // Soft Lavender
                juce::Colour endColor(0xffd4a5a5);    // Dusty Rose
                juce::Colour grainColor = startColor.interpolatedWith(endColor, progress);

                // Draw grain with glow effect
                g.setColour(grainColor.withAlpha(0.2f));
                g.fillEllipse(static_cast<float>(x - radius * 2),
                             static_cast<float>(y - radius * 2),
                             static_cast<float>(radius * 4),
                             static_cast<float>(radius * 4));

                g.setColour(grainColor.withAlpha(0.8f * envelopeSize));
                g.fillEllipse(static_cast<float>(x - radius),
                             static_cast<float>(y - radius),
                             static_cast<float>(radius * 2),
                             static_cast<float>(radius * 2));
            }
        }

        // Show grain count
        g.setColour(juce::Colour(0xfff5f1e8).withAlpha(0.4f));
        g.setFont(9.0f);
        g.drawText(juce::String(activeCount) + " grains",
                  bounds.removeFromBottom(12),
                  juce::Justification::centredRight);
    }

    //==============================================================================
    GrainEngine* grainEngine = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrainVisualizer)
};
