#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * Grains VST - Plugin Editor (UI)
 */
class GrainsAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                     private juce::Slider::Listener,
                                     private juce::Button::Listener,
                                     private juce::FileDragAndDropTarget
{
public:
    GrainsAudioProcessorEditor (GrainsAudioProcessor&);
    ~GrainsAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    // Slider listener
    void sliderValueChanged (juce::Slider* slider) override;

    // Button listener
    void buttonClicked (juce::Button* button) override;

    // Drag and drop
    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

    //==============================================================================
    // Helper to create a styled slider
    void setupSlider (juce::Slider& slider, const juce::String& paramID);
    void setupLabel (juce::Label& label, const juce::String& text);
    void loadAudioFile (const juce::File& file);

    //==============================================================================
    GrainsAudioProcessor& audioProcessor;

    // UI Components
    juce::TextButton loadSampleButton;
    juce::Label sampleLabel;

    // Parameter sliders
    juce::Slider positionSlider;
    juce::Label positionLabel;

    juce::Slider spraySlider;
    juce::Label sprayLabel;

    juce::Slider grainSizeSlider;
    juce::Label grainSizeLabel;

    juce::Slider densitySlider;
    juce::Label densityLabel;

    juce::Slider pitchSlider;
    juce::Label pitchLabel;

    juce::Slider panSlider;
    juce::Label panLabel;

    juce::Slider gainSlider;
    juce::Label gainLabel;

    // Phase 2 controls
    juce::Slider panSpreadSlider;
    juce::Label panSpreadLabel;

    juce::Slider pitchSpreadSlider;
    juce::Label pitchSpreadLabel;

    juce::ToggleButton freezeButton;
    juce::Label freezeLabel;

    juce::Slider reverseSlider;
    juce::Label reverseLabel;

    // Parameter attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> positionAttachment;
    std::unique_ptr<SliderAttachment> sprayAttachment;
    std::unique_ptr<SliderAttachment> grainSizeAttachment;
    std::unique_ptr<SliderAttachment> densityAttachment;
    std::unique_ptr<SliderAttachment> pitchAttachment;
    std::unique_ptr<SliderAttachment> panAttachment;
    std::unique_ptr<SliderAttachment> gainAttachment;

    // Phase 2 attachments
    std::unique_ptr<SliderAttachment> panSpreadAttachment;
    std::unique_ptr<SliderAttachment> pitchSpreadAttachment;
    std::unique_ptr<ButtonAttachment> freezeAttachment;
    std::unique_ptr<SliderAttachment> reverseAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GrainsAudioProcessorEditor)
};
