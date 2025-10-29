#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "GrainEngine.h"

//==============================================================================
/**
 * UnifiedVisualization - Complete waveform + grain + window visualization
 * Similar to web version: shows waveform, grains on waveform, window/spray overlay
 * Grains appear at their actual position on the waveform, vertically positioned by pan
 */
class UnifiedVisualization : public juce::Component,
                              private juce::Timer
{
public:
    UnifiedVisualization()
    {
        startTimerHz(60);  // 60 FPS
    }

    ~UnifiedVisualization() override
    {
        stopTimer();
    }

    //==============================================================================
    void mouseDown(const juce::MouseEvent& event) override
    {
        if (audioBuffer == nullptr || audioBuffer->getNumSamples() == 0)
            return;

        isDraggingPosition = true;
        updatePositionFromMouse(event.position.x);
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        if (isDraggingPosition)
        {
            updatePositionFromMouse(event.position.x);
        }
    }

    void mouseUp(const juce::MouseEvent&) override
    {
        isDraggingPosition = false;
    }

    //==============================================================================
    void setPositionParameter(juce::AudioParameterFloat* param)
    {
        positionParameter = param;
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

        if (audioBuffer == nullptr || audioBuffer->getNumSamples() == 0)
        {
            g.setColour(juce::Colour(0xff9ba4d8).withAlpha(0.5f));
            g.setFont(14.0f);
            g.drawText("No sample loaded", bounds, juce::Justification::centred);
            return;
        }

        auto visualBounds = bounds.reduced(10);

        // Draw waveform first (background)
        drawWaveform(g, visualBounds);

        // Draw window/spray area overlay
        if (grainEngine != nullptr)
        {
            drawWindowOverlay(g, visualBounds);
        }

        // Draw active grains on top
        if (grainEngine != nullptr)
        {
            drawGrains(g, visualBounds);
        }
    }

    //==============================================================================
    void setAudioBuffer(const juce::AudioBuffer<float>* buffer)
    {
        audioBuffer = buffer;
        repaint();
    }

    void setGrainEngine(GrainEngine* engine)
    {
        grainEngine = engine;
    }

private:
    //==============================================================================
    void timerCallback() override
    {
        if (grainEngine != nullptr)
            repaint();
    }

    void drawWaveform(juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        if (audioBuffer == nullptr || audioBuffer->getNumSamples() == 0)
            return;

        const int width = bounds.getWidth();
        const int height = bounds.getHeight();
        const int numSamples = audioBuffer->getNumSamples();
        const float samplesPerPixel = static_cast<float>(numSamples) / static_cast<float>(width);

        // Find the peak value in the buffer for normalization
        float globalPeak = 0.0001f;  // Avoid divide by zero
        for (int i = 0; i < numSamples; ++i)
        {
            float sample = std::abs(audioBuffer->getSample(0, i));
            globalPeak = juce::jmax(globalPeak, sample);
        }

        // Normalize to fill the height (with slight margin)
        const float normalizationFactor = 0.95f / globalPeak;

        juce::Path waveformPath;
        waveformPath.startNewSubPath(static_cast<float>(bounds.getX()), bounds.getCentreY());

        // Draw waveform
        for (int x = 0; x < width; ++x)
        {
            int startSample = static_cast<int>(x * samplesPerPixel);
            int endSample = static_cast<int>((x + 1) * samplesPerPixel);
            if (endSample > numSamples) endSample = numSamples;

            float minVal = 0.0f, maxVal = 0.0f;
            for (int i = startSample; i < endSample; ++i)
            {
                float sample = audioBuffer->getSample(0, i) * normalizationFactor;
                minVal = juce::jmin(minVal, sample);
                maxVal = juce::jmax(maxVal, sample);
            }

            float maxY = (1.0f - (maxVal + 1.0f) * 0.5f) * height + bounds.getY();

            waveformPath.lineTo(static_cast<float>(bounds.getX() + x), maxY);
        }

        // Mirror for bottom half
        for (int x = width - 1; x >= 0; --x)
        {
            int startSample = static_cast<int>(x * samplesPerPixel);
            int endSample = static_cast<int>((x + 1) * samplesPerPixel);
            if (endSample > numSamples) endSample = numSamples;

            float minVal = 0.0f;
            for (int i = startSample; i < endSample; ++i)
            {
                float sample = audioBuffer->getSample(0, i) * normalizationFactor;
                minVal = juce::jmin(minVal, sample);
            }

            float minY = (1.0f - (minVal + 1.0f) * 0.5f) * height + bounds.getY();
            waveformPath.lineTo(static_cast<float>(bounds.getX() + x), minY);
        }

        waveformPath.closeSubPath();

        // Fill waveform with subtle lavender
        g.setColour(juce::Colour(0xffb4a5d8).withAlpha(0.15f));
        g.fillPath(waveformPath);

        // Draw outline with dusty rose
        g.setColour(juce::Colour(0xffd4a5a5).withAlpha(0.6f));
        g.strokePath(waveformPath, juce::PathStrokeType(1.0f));

        // Draw center line
        g.setColour(juce::Colour(0xff4a4a52).withAlpha(0.3f));
        g.drawLine(static_cast<float>(bounds.getX()),
                  bounds.getCentreY(),
                  static_cast<float>(bounds.getRight()),
                  bounds.getCentreY(), 1.0f);
    }

    void drawWindowOverlay(juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        // Get current position and spray from grain engine
        float position = grainEngine->getCurrentPosition();  // This is the position slider value
        float sprayPercent = grainEngine->getCurrentSpray();
        bool isFrozen = grainEngine->isFrozen();

        // Calculate window area on waveform
        float spray01 = sprayPercent / 100.0f;
        float windowStart = juce::jlimit(0.0f, 1.0f, position - spray01);
        float windowEnd = juce::jlimit(0.0f, 1.0f, position + spray01);

        int startX = bounds.getX() + static_cast<int>(windowStart * bounds.getWidth());
        int endX = bounds.getX() + static_cast<int>(windowEnd * bounds.getWidth());

        // Draw window area with subtle lavender glow
        g.setColour(juce::Colour(0xffb4a5d8).withAlpha(0.15f));
        g.fillRect(startX, bounds.getY(), endX - startX, bounds.getHeight());

        // Draw window boundaries
        g.setColour(juce::Colour(0xffb4a5d8).withAlpha(0.5f));
        g.drawLine(static_cast<float>(startX), static_cast<float>(bounds.getY()),
                  static_cast<float>(startX), static_cast<float>(bounds.getBottom()), 1.5f);
        g.drawLine(static_cast<float>(endX), static_cast<float>(bounds.getY()),
                  static_cast<float>(endX), static_cast<float>(bounds.getBottom()), 1.5f);

        // Draw position marker (center)
        int posX = bounds.getX() + static_cast<int>(position * bounds.getWidth());
        g.setColour(juce::Colour(0xffb4a5d8).withAlpha(0.9f));
        g.drawLine(static_cast<float>(posX), static_cast<float>(bounds.getY()),
                  static_cast<float>(posX), static_cast<float>(bounds.getBottom()), 2.0f);

        // If frozen, draw a special indicator
        if (isFrozen)
        {
            g.setColour(juce::Colour(0xffd4a5a5).withAlpha(0.7f));  // Dusty rose for frozen
            g.setFont(11.0f);
            g.drawText("FROZEN", bounds.removeFromTop(20).removeFromRight(60), juce::Justification::centredRight);
        }
    }

    void drawGrains(juce::Graphics& g, juce::Rectangle<int> bounds)
    {
        const auto& voices = grainEngine->getVoices();

        for (const auto& voice : voices)
        {
            if (voice.isActive())
            {
                // Position on x-axis (where in sample)
                float position = voice.getCurrentPosition();
                int x = bounds.getX() + static_cast<int>(position * bounds.getWidth());

                // Position on y-axis based on pan (-1 to +1)
                float pan = voice.getPan();
                // Map pan to vertical position: -1 (left/top) to +1 (right/bottom)
                float yPercent = (pan + 1.0f) * 0.5f;  // 0-1
                int y = bounds.getY() + static_cast<int>(yPercent * bounds.getHeight());

                // Size based on grain progress (envelope)
                float progress = voice.getProgress();
                float envelopeSize = 1.0f - std::abs(progress * 2.0f - 1.0f);  // 0 -> 1 -> 0

                // Size also affected by grain size parameter
                float grainSize = voice.getGrainSize();
                float normalizedSize = juce::jlimit(0.2f, 1.0f, grainSize / 10000.0f);  // Normalize
                int radius = static_cast<int>(4.0f + envelopeSize * normalizedSize * 12.0f);

                // Color gradient based on progress
                juce::Colour startColor(0xffb4a5d8);  // Soft Lavender
                juce::Colour endColor(0xffd4a5a5);    // Dusty Rose
                juce::Colour grainColor = startColor.interpolatedWith(endColor, progress);

                // Draw grain with glow effect
                g.setColour(grainColor.withAlpha(0.3f));
                g.fillEllipse(static_cast<float>(x - radius * 2),
                             static_cast<float>(y - radius * 2),
                             static_cast<float>(radius * 4),
                             static_cast<float>(radius * 4));

                g.setColour(grainColor.withAlpha(0.9f * envelopeSize));
                g.fillEllipse(static_cast<float>(x - radius),
                             static_cast<float>(y - radius),
                             static_cast<float>(radius * 2),
                             static_cast<float>(radius * 2));

                // Optional: draw line connecting grain to its position on waveform center
                g.setColour(grainColor.withAlpha(0.1f));
                g.drawLine(static_cast<float>(x), static_cast<float>(bounds.getCentreY()),
                          static_cast<float>(x), static_cast<float>(y), 1.0f);
            }
        }

        // Draw grain count
        int activeCount = 0;
        for (const auto& voice : voices)
            if (voice.isActive()) activeCount++;

        if (activeCount > 0)
        {
            g.setColour(juce::Colour(0xfff5f1e8).withAlpha(0.5f));
            g.setFont(10.0f);
            g.drawText(juce::String(activeCount) + " grains",
                      bounds.removeFromBottom(12).removeFromRight(80),
                      juce::Justification::centredRight);
        }
    }

    //==============================================================================
    void updatePositionFromMouse(float mouseX)
    {
        if (positionParameter == nullptr)
            return;

        auto bounds = getLocalBounds().reduced(10);
        float position01 = (mouseX - bounds.getX()) / static_cast<float>(bounds.getWidth());
        position01 = juce::jlimit(0.0f, 1.0f, position01);

        // Convert to parameter range (0-100)
        float positionPercent = position01 * 100.0f;
        positionParameter->setValueNotifyingHost(positionParameter->convertTo0to1(positionPercent));
    }

    //==============================================================================
    const juce::AudioBuffer<float>* audioBuffer = nullptr;
    GrainEngine* grainEngine = nullptr;
    juce::AudioParameterFloat* positionParameter = nullptr;
    bool isDraggingPosition = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UnifiedVisualization)
};
