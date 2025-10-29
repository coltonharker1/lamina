#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "GrainEngine.h"

//==============================================================================
/**
 * StereoFieldVisualizer - Shows stereo distribution of active grains
 * Similar to the web version's stereo field visualization
 */
class StereoFieldVisualizer : public juce::Component,
                               private juce::Timer
{
public:
    StereoFieldVisualizer()
    {
        startTimerHz(60);  // 60 FPS for smooth animation
    }

    ~StereoFieldVisualizer() override
    {
        stopTimer();
    }

    //==============================================================================
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();

        // Somnia sleepy theme - darker background
        g.fillAll(juce::Colour(0xff0f1620));

        // Subtle border
        g.setColour(juce::Colour(0xff4a4a52).withAlpha(0.3f));
        g.drawRect(bounds, 1);

        if (grainEngine == nullptr)
        {
            g.setColour(juce::Colour(0xff9ba4d8).withAlpha(0.5f));
            g.setFont(12.0f);
            g.drawText("Stereo Field", bounds, juce::Justification::centred);
            return;
        }

        // Draw stereo field
        drawStereoField(g, bounds);

        // Label
        g.setColour(juce::Colour(0xfff5f1e8).withAlpha(0.6f));
        g.setFont(10.0f);
        g.drawText("STEREO FIELD", bounds.removeFromTop(15), juce::Justification::centred);
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

    void drawStereoField(juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        if (grainEngine == nullptr)
            return;

        bounds = bounds.reduced(15, 20);

        // Draw center line
        int centerX = bounds.getCentreX();
        g.setColour(juce::Colour(0xff4a4a52).withAlpha(0.4f));
        g.drawLine(static_cast<float>(centerX),
                  static_cast<float>(bounds.getY()),
                  static_cast<float>(centerX),
                  static_cast<float>(bounds.getBottom()), 1.0f);

        // Draw L and R markers
        g.setColour(juce::Colour(0xfff5f1e8).withAlpha(0.4f));
        g.setFont(9.0f);
        g.drawText("L", bounds.removeFromLeft(15), juce::Justification::centredLeft);
        g.drawText("R", bounds.removeFromRight(15), juce::Justification::centredRight);

        // Get active voices from grain engine
        const auto& voices = grainEngine->getVoices();

        // Count grains in different pan positions
        std::vector<int> panBuckets(20, 0);  // 20 buckets across stereo field
        int totalActive = 0;

        for (const auto& voice : voices)
        {
            if (voice.isActive())
            {
                totalActive++;

                // Get pan position and convert to bucket index
                float pan = voice.getPan();  // -1 to +1
                int bucketIndex = static_cast<int>((pan + 1.0f) * 0.5f * 19.0f);  // 0-19
                bucketIndex = juce::jlimit(0, 19, bucketIndex);
                panBuckets[bucketIndex]++;
            }
        }

        if (totalActive == 0)
            return;

        // Draw pan distribution bars
        int barWidth = bounds.getWidth() / 20;
        int maxHeight = bounds.getHeight() - 25;

        // Find max value for scaling
        int maxCount = *std::max_element(panBuckets.begin(), panBuckets.end());
        if (maxCount == 0)
            maxCount = 1;

        for (int i = 0; i < 20; ++i)
        {
            if (panBuckets[i] > 0)
            {
                float ratio = static_cast<float>(panBuckets[i]) / static_cast<float>(maxCount);
                int barHeight = static_cast<int>(ratio * maxHeight);

                int x = bounds.getX() + i * barWidth;
                int y = bounds.getBottom() - barHeight;

                // Color gradient from periwinkle (left) through lavender (center) to dusty rose (right)
                float pan01 = i / 19.0f;
                juce::Colour leftColor(0xff9ba4d8);   // Periwinkle
                juce::Colour centerColor(0xffb4a5d8); // Soft Lavender
                juce::Colour rightColor(0xffd4a5a5);  // Dusty Rose

                juce::Colour barColor;
                if (pan01 < 0.5f)
                    barColor = leftColor.interpolatedWith(centerColor, pan01 * 2.0f);
                else
                    barColor = centerColor.interpolatedWith(rightColor, (pan01 - 0.5f) * 2.0f);

                // Draw bar with glow
                g.setColour(barColor.withAlpha(0.3f));
                g.fillRect(x - 1, y - 2, barWidth + 2, barHeight + 4);

                g.setColour(barColor.withAlpha(0.8f));
                g.fillRect(x, y, barWidth, barHeight);
            }
        }

        // Show total grain count
        g.setColour(juce::Colour(0xfff5f1e8).withAlpha(0.4f));
        g.setFont(9.0f);
        g.drawText(juce::String(totalActive) + " active",
                  bounds.removeFromBottom(12),
                  juce::Justification::centred);
    }

    //==============================================================================
    GrainEngine* grainEngine = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StereoFieldVisualizer)
};
