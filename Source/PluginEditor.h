#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * Grains VST - Plugin Editor (UI)
 */
class GrainsAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    GrainsAudioProcessorEditor (GrainsAudioProcessor&);
    ~GrainsAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    GrainsAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GrainsAudioProcessorEditor)
};
