#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "UnifiedVisualization.h"
#include "InteractiveADSREnvelope.h"
#include "ModulatedSlider.h"
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
    class MonoPrintLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPosProportional, float rotaryStartAngle,
                              float rotaryEndAngle, juce::Slider& slider) override;
        void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                  const juce::Colour& backgroundColour,
                                  bool shouldDrawButtonAsHighlighted,
                                  bool shouldDrawButtonAsDown) override;
        void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                            bool shouldDrawButtonAsHighlighted,
                            bool shouldDrawButtonAsDown) override;
        void drawScrollbar(juce::Graphics& g, juce::ScrollBar& scrollbar,
                           int x, int y, int width, int height, bool isScrollbarVertical,
                           int thumbStartPosition, int thumbSize, bool isMouseOver,
                           bool isMouseDown) override;
        juce::Font getTabButtonFont(juce::TabBarButton& button, float height) override;
        void drawTabButton(juce::TabBarButton& button, juce::Graphics& g,
                           bool isMouseOver, bool isMouseDown) override;
        void drawTabButtonText(juce::TabBarButton& button, juce::Graphics& g,
                               bool isMouseOver, bool isMouseDown) override;
    };

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
    MonoPrintLookAndFeel monoPrintLookAndFeel;
    juce::Typeface::Ptr instrumentSerifRegular;
    juce::Typeface::Ptr instrumentSerifItalic;
    juce::Typeface::Ptr jetBrainsMono;

    // UI Components
    juce::TextButton loadSampleButton;

    // Main content component with column borders
    class MainContentComponent : public juce::Component
    {
    public:
        void paint(juce::Graphics& g) override;
        void setSectionPositions(int section1End, int section2End);
        void setColumnBounds(juce::Rectangle<int> grainDesign, juce::Rectangle<int> filters, juce::Rectangle<int> output);

    private:
        int section1EndY = 0;
        int section2EndY = 0;
        juce::Rectangle<int> grainDesignBounds;
        juce::Rectangle<int> filtersBounds;
        juce::Rectangle<int> outputBounds;
    };

    // Main content viewport (for scrolling on smaller displays)
    juce::Viewport mainViewport;
    MainContentComponent mainContentComponent;

    UnifiedVisualization unifiedVisualization;

    // Section labels
    juce::Label grainSourceSectionLabel;
    juce::Label outputSectionLabel;

    // Parameter sliders (using ModulatedSlider for LFO-modulatable parameters)
    ModulatedSlider positionSlider;
    juce::Label positionLabel;

    ModulatedSlider spraySlider;
    juce::Label sprayLabel;

    ModulatedSlider grainSizeSlider;
    juce::Label grainSizeLabel;

    ModulatedSlider densitySlider;
    juce::Label densityLabel;

    ModulatedSlider pitchSlider;
    juce::Label pitchLabel;

    ModulatedSlider panSlider;
    juce::Label panLabel;

    juce::Slider gainSlider;
    juce::Label gainLabel;

    // Phase 2 controls
    ModulatedSlider panSpreadSlider;
    juce::Label panSpreadLabel;

    juce::ToggleButton reverseToggle;
    juce::Label reverseLabel;

    // Interactive ADSR Envelope (replaces separate knobs + visualizer)
    InteractiveADSREnvelope interactiveADSR;

    // Filter controls
    juce::Label filterSectionLabel;

    // Low-pass filter
    juce::ToggleButton lpFilterToggle;
    juce::Label lpFilterLabel;
    juce::Slider lpFilterCutoffSlider;
    juce::Label lpFilterCutoffLabel;
    juce::Slider lpFilterResonanceSlider;
    juce::Label lpFilterResonanceLabel;

    // High-pass filter
    juce::ToggleButton hpFilterToggle;
    juce::Label hpFilterLabel;
    juce::Slider hpFilterCutoffSlider;
    juce::Label hpFilterCutoffLabel;
    juce::Slider hpFilterResonanceSlider;
    juce::Label hpFilterResonanceLabel;

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
        juce::Slider octaveSpreadSlider;
        juce::Label octaveSpreadLabel;
        juce::Slider octaveProbabilitySlider;
        juce::Label octaveProbabilityLabel;
        juce::Slider thirdOctaveProbSlider;
        juce::Label thirdOctaveProbLabel;
        juce::ComboBox grainShapeCombo;
        juce::Label grainShapeLabel;

        // Sound Quality / Randomization controls (ported from grains-vst)
        juce::Slider filterRandomizationSlider;
        juce::Label filterRandomizationLabel;
        juce::Slider detuneSlider;
        juce::Label detuneLabel;
        juce::Slider jitterSlider;
        juce::Label jitterLabel;
        juce::Slider sizeRandomizationSlider;
        juce::Label sizeRandomizationLabel;

        // Section headers for Sound Design tab
        juce::Label playbackSectionLabel;
        juce::Label octavesSectionLabel;
        juce::Label soundQualitySectionLabel;


        // Close button
        juce::TextButton closeButton;

        // Tabbed component
        juce::TabbedComponent tabbedComponent;

        // Tab content components
        class ModulationTab : public juce::Component
        {
        public:
            ModulationTab(AdvancedPanel& parent);
            void resized() override;
        private:
            AdvancedPanel& panel;
            juce::Viewport viewport;
            juce::Component contentComponent;
            bool isResizing = false;
        };

        class SoundDesignTab : public juce::Component
        {
        public:
            SoundDesignTab(AdvancedPanel& parent);
            void resized() override;

            // Store divider positions for painting
            int divider1Y = 0;
            int divider2Y = 0;

        private:
            AdvancedPanel& panel;
            juce::Viewport viewport;

            // Custom content component that draws section dividers
            class ContentWithDividers : public juce::Component
            {
            public:
                ContentWithDividers(SoundDesignTab& tab) : parentTab(tab) {}
                void paint(juce::Graphics& g) override
                {
                    // Draw horizontal divider lines
                    g.setColour(juce::Colour(0xff050505).withAlpha(0.22f));

                    if (parentTab.divider1Y > 0)
                    {
                        g.drawLine(20.0f, static_cast<float>(parentTab.divider1Y),
                                   static_cast<float>(getWidth() - 20), static_cast<float>(parentTab.divider1Y), 1.0f);
                    }
                    if (parentTab.divider2Y > 0)
                    {
                        g.drawLine(20.0f, static_cast<float>(parentTab.divider2Y),
                                   static_cast<float>(getWidth() - 20), static_cast<float>(parentTab.divider2Y), 1.0f);
                    }
                }
            private:
                SoundDesignTab& parentTab;
            };

            ContentWithDividers contentComponent { *this };
            bool isResizing = false;
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
    std::unique_ptr<SliderAttachment> octaveSpreadAttachment;  // In advanced panel Sound Design tab
    std::unique_ptr<SliderAttachment> octaveProbabilityAttachment;  // In advanced panel Sound Design tab
    std::unique_ptr<SliderAttachment> thirdOctaveProbAttachment;  // In advanced panel Sound Design tab

    // Sound quality / randomization attachments (ported from grains-vst)
    std::unique_ptr<SliderAttachment> filterRandomizationAttachment;
    std::unique_ptr<SliderAttachment> detuneAttachment;
    std::unique_ptr<SliderAttachment> jitterAttachment;
    std::unique_ptr<SliderAttachment> sizeRandomizationAttachment;

    // Filter attachments
    std::unique_ptr<ButtonAttachment> lpFilterToggleAttachment;
    std::unique_ptr<SliderAttachment> lpFilterCutoffAttachment;
    std::unique_ptr<SliderAttachment> lpFilterResonanceAttachment;
    std::unique_ptr<ButtonAttachment> hpFilterToggleAttachment;
    std::unique_ptr<SliderAttachment> hpFilterCutoffAttachment;
    std::unique_ptr<SliderAttachment> hpFilterResonanceAttachment;

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
    void updateModulationIndicators();

    // Timer callback for visual feedback
    void timerCallback() override;

    // Track which sliders are being dragged
    std::set<juce::Slider*> slidersBeingDragged;

    // Drag and drop state
    bool isDraggingFile = false;

    // Resize state tracking to prevent jittering
    bool isResizing = false;
    juce::Rectangle<int> lastBounds;

    // Tooltip window for displaying tooltips
    juce::TooltipWindow tooltipWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GrainsAudioProcessorEditor)
};
