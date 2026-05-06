#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>

//==============================================================================
/**
 * WaveformDisplay - Displays the loaded audio sample as a waveform
 */
class WaveformDisplay : public juce::Component,
                        private juce::Timer
{
public:
    WaveformDisplay()
    {
        startTimerHz(30);  // 30 FPS refresh rate
    }

    ~WaveformDisplay() override
    {
        stopTimer();
    }

    //==============================================================================
    void paint(juce::Graphics& g) override
    {
        // Lamina sleepy theme - darker midnight blue background
        g.fillAll(juce::Colour(0xff0f1620));

        // Subtle warm charcoal border
        g.setColour(juce::Colour(0xff4a4a52).withAlpha(0.3f));
        g.drawRect(getLocalBounds(), 1);

        if (audioBuffer == nullptr || audioBuffer->getNumSamples() == 0)
        {
            // No sample loaded - show message in periwinkle
            g.setColour(juce::Colour(0xff9ba4d8).withAlpha(0.5f));
            g.setFont(14.0f);
            g.drawText("No sample loaded", getLocalBounds(), juce::Justification::centred);
            return;
        }

        // Draw waveform
        drawWaveform(g);

        // Draw position marker if available - soft lavender glow
        if (playbackPosition >= 0.0f && playbackPosition <= 1.0f)
        {
            int markerX = static_cast<int>(playbackPosition * getWidth());
            g.setColour(juce::Colour(0xffb4a5d8).withAlpha(0.8f));
            g.drawLine(static_cast<float>(markerX), 0.0f,
                      static_cast<float>(markerX), static_cast<float>(getHeight()), 2.0f);
        }
    }

    //==============================================================================
    /** Set the audio buffer to display */
    void setAudioBuffer(const juce::AudioBuffer<float>* buffer)
    {
        audioBuffer = buffer;
        repaint();
    }

    /** Set playback position (0-1) to show where grains are spawning */
    void setPlaybackPosition(float position)
    {
        playbackPosition = position;
    }

private:
    //==============================================================================
    void timerCallback() override
    {
        // Trigger repaint for smooth position updates
        if (audioBuffer != nullptr)
            repaint();
    }

    void drawWaveform(juce::Graphics& g)
    {
        if (audioBuffer == nullptr || audioBuffer->getNumSamples() == 0)
            return;

        const int width = getWidth();
        const int height = getHeight();
        const int numSamples = audioBuffer->getNumSamples();
        const int channel = 0;  // Use first channel (mono)

        // Calculate how many samples per pixel
        const float samplesPerPixel = static_cast<float>(numSamples) / static_cast<float>(width);

        // Draw waveform
        juce::Path waveformPath;
        waveformPath.startNewSubPath(0.0f, height / 2.0f);

        for (int x = 0; x < width; ++x)
        {
            // Get range of samples for this pixel
            int startSample = static_cast<int>(x * samplesPerPixel);
            int endSample = static_cast<int>((x + 1) * samplesPerPixel);

            if (endSample > numSamples)
                endSample = numSamples;

            // Find min and max in this range
            float minVal = 0.0f;
            float maxVal = 0.0f;

            for (int i = startSample; i < endSample; ++i)
            {
                float sample = audioBuffer->getSample(channel, i);
                minVal = juce::jmin(minVal, sample);
                maxVal = juce::jmax(maxVal, sample);
            }

            // Convert to screen coordinates
            float minY = (1.0f - (minVal + 1.0f) * 0.5f) * height;
            float maxY = (1.0f - (maxVal + 1.0f) * 0.5f) * height;

            // Draw vertical line for this pixel
            waveformPath.lineTo(static_cast<float>(x), maxY);
        }

        // Mirror for bottom half
        for (int x = width - 1; x >= 0; --x)
        {
            int startSample = static_cast<int>(x * samplesPerPixel);
            int endSample = static_cast<int>((x + 1) * samplesPerPixel);

            if (endSample > numSamples)
                endSample = numSamples;

            float minVal = 0.0f;
            for (int i = startSample; i < endSample; ++i)
            {
                float sample = audioBuffer->getSample(channel, i);
                minVal = juce::jmin(minVal, sample);
            }

            float minY = (1.0f - (minVal + 1.0f) * 0.5f) * height;
            waveformPath.lineTo(static_cast<float>(x), minY);
        }

        waveformPath.closeSubPath();

        // Fill waveform with soft lavender glow - Lamina theme
        g.setColour(juce::Colour(0xffb4a5d8).withAlpha(0.25f));
        g.fillPath(waveformPath);

        // Draw outline with dusty rose accent
        g.setColour(juce::Colour(0xffd4a5a5).withAlpha(0.8f));
        g.strokePath(waveformPath, juce::PathStrokeType(1.0f));
    }

    //==============================================================================
    const juce::AudioBuffer<float>* audioBuffer = nullptr;
    float playbackPosition = -1.0f;  // -1 means don't show

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};
