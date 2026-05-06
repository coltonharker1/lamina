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

        constexpr juce::uint32 paper = 0xfff4f1ea;
        constexpr juce::uint32 ink = 0xff050505;
        constexpr juce::uint32 quietInk = 0xaa000000;
        constexpr juce::uint32 faintInk = 0x14000000;

        g.fillAll(juce::Colour(paper));
        g.setColour(juce::Colour(ink));
        g.drawRect(bounds, 1);

        auto header = bounds.removeFromTop(30);
        g.drawLine(static_cast<float>(header.getX()), static_cast<float>(header.getBottom()) - 0.5f,
                   static_cast<float>(header.getRight()), static_cast<float>(header.getBottom()) - 0.5f, 1.0f);
        g.setFont(juce::Font(juce::FontOptions(8.0f)));
        g.drawText("FIG. I - SOUND SPECIMEN", header.reduced(12, 0), juce::Justification::centredLeft);

        for (int x = bounds.getX(); x < bounds.getRight(); x += 64)
        {
            g.setColour(juce::Colour(faintInk));
            g.drawLine(static_cast<float>(x), static_cast<float>(bounds.getY()),
                       static_cast<float>(x), static_cast<float>(bounds.getBottom()), 0.7f);
        }

        if (audioBuffer == nullptr || audioBuffer->getNumSamples() == 0)
        {
            g.setColour(juce::Colour(quietInk));
            g.setFont(juce::Font(juce::FontOptions(20.0f)));
            g.drawText("No specimen loaded", bounds, juce::Justification::centred);
            return;
        }

        auto visualBounds = bounds.reduced(14, 12);

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

        g.setColour(juce::Colour(0xff050505).withAlpha(0.16f));
        g.fillPath(waveformPath);

        g.setColour(juce::Colour(0xff050505).withAlpha(0.72f));
        g.strokePath(waveformPath, juce::PathStrokeType(0.9f));

        g.setColour(juce::Colour(0xff050505).withAlpha(0.20f));
        g.drawLine(static_cast<float>(bounds.getX()),
                  bounds.getCentreY(),
                  static_cast<float>(bounds.getRight()),
                  bounds.getCentreY(), 1.0f);

        g.setColour(juce::Colour(0xff050505));
        g.setFont(juce::Font(juce::FontOptions(7.0f)));
        g.drawText("0.0 S", bounds.withHeight(14).withY(bounds.getBottom() - 14), juce::Justification::centredLeft);
        g.drawText(juce::String(static_cast<double>(numSamples) / 48000.0, 1) + " S",
                   bounds.withHeight(14).withY(bounds.getBottom() - 14), juce::Justification::centredRight);
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

        juce::Rectangle<int> sprayWindow(startX, bounds.getY() + 6,
                                         juce::jmax(1, endX - startX), bounds.getHeight() - 12);
        g.setColour(juce::Colour(0xff050505).withAlpha(0.045f));
        g.fillRect(sprayWindow);

        g.setColour(juce::Colour(0xff050505).withAlpha(0.55f));
        g.drawLine(static_cast<float>(startX), static_cast<float>(bounds.getY()),
                  static_cast<float>(startX), static_cast<float>(bounds.getBottom()), 1.5f);
        g.drawLine(static_cast<float>(endX), static_cast<float>(bounds.getY()),
                  static_cast<float>(endX), static_cast<float>(bounds.getBottom()), 1.5f);

        int posX = bounds.getX() + static_cast<int>(position * bounds.getWidth());
        g.setColour(juce::Colour(0xff050505));
        g.drawLine(static_cast<float>(posX), static_cast<float>(bounds.getY()),
                  static_cast<float>(posX), static_cast<float>(bounds.getBottom()), 2.0f);
        g.setFont(juce::Font(juce::FontOptions(7.0f)));
        g.drawText("POS " + juce::String(position * 100.0f, 0) + "%",
                   juce::Rectangle<int>(posX + 4, bounds.getY() + 4, 80, 12),
                   juce::Justification::centredLeft);

        if (isFrozen)
        {
            g.setColour(juce::Colour(0xff050505).withAlpha(0.72f));
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

                const float bloomRadius = static_cast<float>(radius) * (0.62f + envelopeSize * 0.35f);
                const float wobble = 0.18f + 0.08f * std::sin(position * 47.0f + progress * 6.0f);

                juce::Path blot;
                for (int p = 0; p < 10; ++p)
                {
                    const float angle = juce::MathConstants<float>::twoPi * static_cast<float>(p) / 10.0f;
                    const float irregularity = 1.0f + wobble * std::sin(angle * 3.0f + position * 19.0f)
                                             + 0.08f * std::cos(angle * 5.0f + progress * 11.0f);
                    const float px = static_cast<float>(x) + std::cos(angle) * bloomRadius * irregularity;
                    const float py = static_cast<float>(y) + std::sin(angle) * bloomRadius * irregularity;

                    if (p == 0)
                        blot.startNewSubPath(px, py);
                    else
                        blot.lineTo(px, py);
                }
                blot.closeSubPath();

                juce::Path halo;
                for (int p = 0; p < 12; ++p)
                {
                    const float angle = juce::MathConstants<float>::twoPi * static_cast<float>(p) / 12.0f;
                    const float irregularity = 1.0f + wobble * 0.7f * std::cos(angle * 4.0f + position * 23.0f);
                    const float px = static_cast<float>(x) + std::cos(angle) * bloomRadius * 1.85f * irregularity;
                    const float py = static_cast<float>(y) + std::sin(angle) * bloomRadius * 1.55f * irregularity;

                    if (p == 0)
                        halo.startNewSubPath(px, py);
                    else
                        halo.lineTo(px, py);
                }
                halo.closeSubPath();

                g.setColour(juce::Colour(0xff050505).withAlpha(0.075f * envelopeSize));
                g.fillPath(halo);
                g.setColour(juce::Colour(0xff050505).withAlpha(0.48f * envelopeSize));
                g.fillPath(blot);
                g.setColour(juce::Colour(0xff050505).withAlpha(0.72f * envelopeSize));
                g.fillEllipse(static_cast<float>(x) - bloomRadius * 0.38f,
                              static_cast<float>(y) - bloomRadius * 0.34f,
                              bloomRadius * 0.76f,
                              bloomRadius * 0.68f);

                g.setColour(juce::Colour(0xff050505).withAlpha(0.13f));
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
            g.setColour(juce::Colour(0xff050505).withAlpha(0.64f));
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
