#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "UnifiedVisualization.h"
#include <set>

//==============================================================================
/**
 * Grains VST - Plugin Editor (UI)
 */
class GrainsAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                     private juce::Slider::Listener,
                                     private juce::Button::Listener,
                                     private juce::FileDragAndDropTarget,
                                     private juce::Timer
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
    void sliderDragStarted (juce::Slider* slider) override;
    void sliderDragEnded (juce::Slider* slider) override;

    // Button listener
    void buttonClicked (juce::Button* button) override;

    // Drag and drop
    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;
    void fileDragEnter (const juce::StringArray& files, int x, int y) override;
    void fileDragExit (const juce::StringArray& files) override;

    //==============================================================================
    // Helper to create a styled slider
    void setupSlider (juce::Slider& slider, const juce::String& paramID);
    void setupLabel (juce::Label& label, const juce::String& text);
    void loadAudioFile (const juce::File& file);

    //==============================================================================
    GrainsAudioProcessor& audioProcessor;

    // UI Components
    juce::TextButton loadSampleButton;

    // Main content component with section dividers
    class MainContentComponent : public juce::Component
    {
    public:
        void paint(juce::Graphics& g) override;
        void setSectionPositions(int section1End, int section2End);

    private:
        int section1EndY = 0;
        int section2EndY = 0;
    };

    // Main content viewport (for scrolling on smaller displays)
    juce::Viewport mainViewport;
    MainContentComponent mainContentComponent;

    UnifiedVisualization unifiedVisualization;

    // Section labels
    juce::Label grainSourceSectionLabel;
    juce::Label outputSectionLabel;

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

    juce::ToggleButton reverseToggle;
    juce::Label reverseLabel;

    // Phase 5 controls - Envelope
    juce::Slider envAttackSlider;
    juce::Label envAttackLabel;
    juce::Slider envDecaySlider;
    juce::Label envDecayLabel;
    juce::Slider envSustainSlider;
    juce::Label envSustainLabel;
    juce::Slider envReleaseSlider;
    juce::Label envReleaseLabel;

    juce::ComboBox grainShapeCombo;
    juce::Label grainShapeLabel;

    // Phase 4 controls - LFO System (in advanced panel overlay)
    juce::TextButton advancedToggleButton;
    bool showAdvancedPanel = false;

    // Advanced panel overlay component
    class AdvancedPanel : public juce::Component
    {
    public:
        AdvancedPanel();
        void paint(juce::Graphics& g) override;
        void resized() override;

        struct LFOControls
        {
            juce::Slider rateSlider;
            juce::Label rateLabel;
            juce::Slider depthSlider;
            juce::Label depthLabel;
            juce::ToggleButton enabledToggle;
            juce::ComboBox waveformCombo;
            juce::Label waveformLabel;
            juce::ComboBox targetCombo;
            juce::Label targetLabel;
        };
        std::array<LFOControls, 4> lfoControls;

        // Sound Design controls
        juce::Slider reverseProbabilitySlider;
        juce::Label reverseProbabilityLabel;
        juce::Slider timeStretchSlider;
        juce::Label timeStretchLabel;
        juce::ToggleButton timeStretchToggle;
        juce::Label timeStretchToggleLabel;

        // Close button
        juce::TextButton closeButton;

        // Tabbed component
        juce::TabbedComponent tabbedComponent;

        // Tab content components
        class ModulationTab : public juce::Component
        {
        public:
            ModulationTab(AdvancedPanel& parent) : panel(parent) {}
            void resized() override;
        private:
            AdvancedPanel& panel;
        };

        class SoundDesignTab : public juce::Component
        {
        public:
            SoundDesignTab(AdvancedPanel& parent) : panel(parent) {}
            void resized() override;
        private:
            AdvancedPanel& panel;
        };

        std::unique_ptr<ModulationTab> modulationTab;
        std::unique_ptr<SoundDesignTab> soundDesignTab;
    };

    std::unique_ptr<AdvancedPanel> advancedPanel;

    // Parameter attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SliderAttachment> positionAttachment;
    std::unique_ptr<SliderAttachment> sprayAttachment;
    std::unique_ptr<SliderAttachment> grainSizeAttachment;
    std::unique_ptr<SliderAttachment> densityAttachment;
    std::unique_ptr<SliderAttachment> pitchAttachment;
    std::unique_ptr<SliderAttachment> panAttachment;
    std::unique_ptr<SliderAttachment> gainAttachment;

    // Phase 2 attachments
    std::unique_ptr<SliderAttachment> panSpreadAttachment;
    std::unique_ptr<ButtonAttachment> reverseAttachment;

    // Phase 3 attachments
    std::unique_ptr<ComboBoxAttachment> grainShapeAttachment;

    // Advanced panel attachments
    std::unique_ptr<SliderAttachment> timeStretchAttachment;  // In advanced panel Sound Design tab
    std::unique_ptr<ButtonAttachment> timeStretchToggleAttachment;  // In advanced panel Sound Design tab
    std::unique_ptr<SliderAttachment> reverseProbabilityAttachment;  // In advanced panel Sound Design tab

    // Phase 5 attachments - Envelope
    std::unique_ptr<SliderAttachment> envAttackAttachment;
    std::unique_ptr<SliderAttachment> envDecayAttachment;
    std::unique_ptr<SliderAttachment> envSustainAttachment;
    std::unique_ptr<SliderAttachment> envReleaseAttachment;

    // Phase 4 attachments - LFO System (stored in editor, not in panel)
    struct LFOAttachments
    {
        std::unique_ptr<SliderAttachment> rateAttachment;
        std::unique_ptr<SliderAttachment> depthAttachment;
        std::unique_ptr<ButtonAttachment> enabledAttachment;
        std::unique_ptr<ComboBoxAttachment> waveformAttachment;
        std::unique_ptr<ComboBoxAttachment> targetAttachment;
    };
    std::array<LFOAttachments, 4> lfoAttachments;

    // Helper methods
    void toggleAdvancedPanel();
    void updateTimeStretchSliderState();
    void updateVisualFeedback();

    // Timer callback for visual feedback
    void timerCallback() override;

    // Track which sliders are being dragged
    std::set<juce::Slider*> slidersBeingDragged;

    // Drag and drop state
    bool isDraggingFile = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GrainsAudioProcessorEditor)
};
