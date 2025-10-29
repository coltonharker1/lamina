#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// MainContentComponent Implementation
//==============================================================================
void GrainsAudioProcessorEditor::MainContentComponent::paint(juce::Graphics& g)
{
    // Draw section dividers
    g.setColour(juce::Colour(0xff9ba4d8).withAlpha(0.15f));

    if (section1EndY > 0)
        g.drawLine(20.0f, static_cast<float>(section1EndY), static_cast<float>(getWidth()) - 20.0f, static_cast<float>(section1EndY), 1.0f);

    if (section2EndY > 0)
        g.drawLine(20.0f, static_cast<float>(section2EndY), static_cast<float>(getWidth()) - 20.0f, static_cast<float>(section2EndY), 1.0f);
}

void GrainsAudioProcessorEditor::MainContentComponent::setSectionPositions(int section1End, int section2End)
{
    section1EndY = section1End;
    section2EndY = section2End;
    repaint();
}

//==============================================================================
// Advanced Panel Implementation
//==============================================================================
GrainsAudioProcessorEditor::AdvancedPanel::AdvancedPanel()
    : tabbedComponent(juce::TabbedButtonBar::TabsAtTop)
{
    // Setup close button
    closeButton.setButtonText("X");
    closeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a4a52));
    closeButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff5f1e8));
    addAndMakeVisible(closeButton);

    // Setup tabbed component
    tabbedComponent.setTabBarDepth(35);
    tabbedComponent.setColour(juce::TabbedComponent::backgroundColourId, juce::Colour(0xff1a2332));
    tabbedComponent.setColour(juce::TabbedComponent::outlineColourId, juce::Colour(0xff4a4a52));
    tabbedComponent.setColour(juce::TabbedButtonBar::tabOutlineColourId, juce::Colour(0xff4a4a52));
    tabbedComponent.setColour(juce::TabbedButtonBar::frontOutlineColourId, juce::Colour(0xffb4a5d8));
    tabbedComponent.setColour(juce::TabbedButtonBar::tabTextColourId, juce::Colour(0xff9ba4d8));
    tabbedComponent.setColour(juce::TabbedButtonBar::frontTextColourId, juce::Colour(0xfff5f1e8));
    addAndMakeVisible(tabbedComponent);

    // Create tab content components
    modulationTab = std::make_unique<ModulationTab>(*this);
    soundDesignTab = std::make_unique<SoundDesignTab>(*this);

    // Add tabs
    tabbedComponent.addTab("Modulation", juce::Colour(0xff1a2332), modulationTab.get(), false);
    tabbedComponent.addTab("Sound Design", juce::Colour(0xff1a2332), soundDesignTab.get(), false);

    // Setup LFO controls (added to Modulation tab)
    for (int i = 0; i < 4; ++i)
    {
        auto& controls = lfoControls[i];
        juce::String lfoNum = juce::String(i + 1);

        // Rate slider
        controls.rateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        controls.rateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        controls.rateSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffb4a5d8));
        controls.rateSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff5f1e8));
        controls.rateSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff4a4a52));
        modulationTab->addAndMakeVisible(controls.rateSlider);

        controls.rateLabel.setText("LFO" + lfoNum + " Rate", juce::dontSendNotification);
        controls.rateLabel.setFont(juce::Font(juce::FontOptions("Arial", 12.0f, juce::Font::plain)));
        controls.rateLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ba4d8));
        controls.rateLabel.setJustificationType(juce::Justification::centred);
        modulationTab->addAndMakeVisible(controls.rateLabel);

        // Depth slider
        controls.depthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        controls.depthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        controls.depthSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffb4a5d8));
        controls.depthSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff5f1e8));
        controls.depthSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff4a4a52));
        modulationTab->addAndMakeVisible(controls.depthSlider);

        controls.depthLabel.setText("LFO" + lfoNum + " Depth", juce::dontSendNotification);
        controls.depthLabel.setFont(juce::Font(juce::FontOptions("Arial", 12.0f, juce::Font::plain)));
        controls.depthLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ba4d8));
        controls.depthLabel.setJustificationType(juce::Justification::centred);
        modulationTab->addAndMakeVisible(controls.depthLabel);

        // Enabled toggle button
        controls.enabledToggle.setButtonText("ON");
        controls.enabledToggle.setColour(juce::ToggleButton::textColourId, juce::Colour(0xfff5f1e8));
        controls.enabledToggle.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xffb4a5d8));
        controls.enabledToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xff4a4a52));
        modulationTab->addAndMakeVisible(controls.enabledToggle);

        // Waveform combo
        controls.waveformCombo.addItem("Sine", 1);
        controls.waveformCombo.addItem("Triangle", 2);
        controls.waveformCombo.addItem("Saw Up", 3);
        controls.waveformCombo.addItem("Saw Down", 4);
        controls.waveformCombo.addItem("Square", 5);
        controls.waveformCombo.addItem("Random", 6);
        controls.waveformCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1a2332));
        controls.waveformCombo.setColour(juce::ComboBox::textColourId, juce::Colour(0xfff5f1e8));
        controls.waveformCombo.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff4a4a52));
        controls.waveformCombo.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xffb4a5d8));
        modulationTab->addAndMakeVisible(controls.waveformCombo);

        controls.waveformLabel.setText("Waveform", juce::dontSendNotification);
        controls.waveformLabel.setFont(juce::Font(juce::FontOptions("Arial", 12.0f, juce::Font::plain)));
        controls.waveformLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ba4d8));
        controls.waveformLabel.setJustificationType(juce::Justification::centred);
        modulationTab->addAndMakeVisible(controls.waveformLabel);

        // Target combo
        controls.targetCombo.addItem("Position", 1);
        controls.targetCombo.addItem("Spray", 2);
        controls.targetCombo.addItem("Size", 3);
        controls.targetCombo.addItem("Density", 4);
        controls.targetCombo.addItem("Pitch", 5);
        controls.targetCombo.addItem("Pan", 6);
        controls.targetCombo.addItem("Pan Spread", 7);
        controls.targetCombo.addItem("Reverse", 8);
        controls.targetCombo.addItem("None", 9);
        controls.targetCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1a2332));
        controls.targetCombo.setColour(juce::ComboBox::textColourId, juce::Colour(0xfff5f1e8));
        controls.targetCombo.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff4a4a52));
        controls.targetCombo.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xffb4a5d8));
        modulationTab->addAndMakeVisible(controls.targetCombo);

        controls.targetLabel.setText("Target", juce::dontSendNotification);
        controls.targetLabel.setFont(juce::Font(juce::FontOptions("Arial", 12.0f, juce::Font::plain)));
        controls.targetLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ba4d8));
        controls.targetLabel.setJustificationType(juce::Justification::centred);
        modulationTab->addAndMakeVisible(controls.targetLabel);
    }

    // Setup Sound Design controls
    // Time Stretch slider
    timeStretchSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    timeStretchSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    timeStretchSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffb4a5d8));
    timeStretchSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff5f1e8));
    timeStretchSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff4a4a52));
    soundDesignTab->addAndMakeVisible(timeStretchSlider);

    timeStretchLabel.setText("Time", juce::dontSendNotification);
    timeStretchLabel.setFont(juce::Font(juce::FontOptions("Arial", 13.0f, juce::Font::plain)));
    timeStretchLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ba4d8));
    timeStretchLabel.setJustificationType(juce::Justification::centred);
    soundDesignTab->addAndMakeVisible(timeStretchLabel);

    // Time Stretch toggle (pitch/time lock)
    timeStretchToggle.setButtonText("LOCK");
    timeStretchToggle.setColour(juce::ToggleButton::textColourId, juce::Colour(0xfff5f1e8));
    timeStretchToggle.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xffb4a5d8));
    timeStretchToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xff4a4a52));
    soundDesignTab->addAndMakeVisible(timeStretchToggle);

    timeStretchToggleLabel.setText("Pitch/Time Lock", juce::dontSendNotification);
    timeStretchToggleLabel.setFont(juce::Font(juce::FontOptions("Arial", 13.0f, juce::Font::plain)));
    timeStretchToggleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ba4d8));
    timeStretchToggleLabel.setJustificationType(juce::Justification::centred);
    soundDesignTab->addAndMakeVisible(timeStretchToggleLabel);

    // Reverse Probability slider
    reverseProbabilitySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    reverseProbabilitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    reverseProbabilitySlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffb4a5d8));
    reverseProbabilitySlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff5f1e8));
    reverseProbabilitySlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff4a4a52));
    soundDesignTab->addAndMakeVisible(reverseProbabilitySlider);

    reverseProbabilityLabel.setText("Reverse Probability", juce::dontSendNotification);
    reverseProbabilityLabel.setFont(juce::Font(juce::FontOptions("Arial", 13.0f, juce::Font::plain)));
    reverseProbabilityLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ba4d8));
    reverseProbabilityLabel.setJustificationType(juce::Justification::centred);
    soundDesignTab->addAndMakeVisible(reverseProbabilityLabel);
}

void GrainsAudioProcessorEditor::AdvancedPanel::paint(juce::Graphics& g)
{
    // Semi-transparent dark background
    g.fillAll(juce::Colour(0xff0f1620).withAlpha(0.95f));

    // Border on the right side
    g.setColour(juce::Colour(0xffb4a5d8).withAlpha(0.3f));
    g.drawLine(static_cast<float>(getWidth()), 0, static_cast<float>(getWidth()), static_cast<float>(getHeight()), 2.0f);
}

void GrainsAudioProcessorEditor::AdvancedPanel::resized()
{
    // Close button in top left
    closeButton.setBounds(10, 10, 40, 30);

    // Tabbed component takes remaining space
    auto tabbedBounds = getLocalBounds();
    tabbedBounds.removeFromTop(50);  // Leave space for close button
    tabbedComponent.setBounds(tabbedBounds);
}

//==============================================================================
// ModulationTab Implementation
//==============================================================================
void GrainsAudioProcessorEditor::AdvancedPanel::ModulationTab::resized()
{
    const int margin = 20;
    const int knobSize = 70;  // Increased from 50px for better visibility
    const int comboWidth = 230;
    const int labelHeight = 14;
    const int lfoBlockHeight = 200;  // Increased to accommodate larger knobs
    const int columnWidth = (getWidth() - 3 * margin) / 2;

    int yPos = margin;

    // Layout LFOs in 2 columns, 2 rows
    for (int i = 0; i < 4; ++i)
    {
        auto& controls = panel.lfoControls[i];

        int col = i % 2;
        int row = i / 2;

        int xBase = margin + col * (columnWidth + margin);
        int yBase = yPos + row * (lfoBlockHeight + 15);
        int yControl = yBase;

        // Rate slider
        controls.rateLabel.setBounds(xBase, yControl, 80, labelHeight);
        controls.rateSlider.setBounds(xBase + 5, yControl + labelHeight, knobSize, knobSize);

        // Depth slider
        controls.depthLabel.setBounds(xBase + 90, yControl, 80, labelHeight);
        controls.depthSlider.setBounds(xBase + 95, yControl + labelHeight, knobSize, knobSize);

        // Enabled toggle
        controls.enabledToggle.setBounds(xBase + 160, yControl + labelHeight + 15, 50, 24);

        yControl += knobSize + labelHeight + 10;

        // Waveform dropdown
        controls.waveformLabel.setBounds(xBase, yControl, comboWidth, labelHeight);
        controls.waveformCombo.setBounds(xBase, yControl + labelHeight + 2, comboWidth, 24);

        yControl += labelHeight + 24 + 8;

        // Target dropdown
        controls.targetLabel.setBounds(xBase, yControl, comboWidth, labelHeight);
        controls.targetCombo.setBounds(xBase, yControl + labelHeight + 2, comboWidth, 24);
    }
}

//==============================================================================
// SoundDesignTab Implementation
//==============================================================================
void GrainsAudioProcessorEditor::AdvancedPanel::SoundDesignTab::resized()
{
    const int margin = 30;
    const int knobSize = 80;
    const int labelHeight = 16;
    const int verticalSpacing = 120;

    int yPos = margin + 20;

    // Time Stretch - centered
    panel.timeStretchLabel.setBounds((getWidth() - 120) / 2, yPos, 120, labelHeight);
    panel.timeStretchSlider.setBounds((getWidth() - knobSize) / 2, yPos + labelHeight + 5, knobSize, knobSize);

    yPos += labelHeight + knobSize + 10;

    // Time Stretch Toggle - below slider
    panel.timeStretchToggleLabel.setBounds((getWidth() - 120) / 2, yPos, 120, labelHeight);
    panel.timeStretchToggle.setBounds((getWidth() - 70) / 2, yPos + labelHeight + 5, 70, 30);

    yPos += verticalSpacing;

    // Reverse Probability - centered
    panel.reverseProbabilityLabel.setBounds((getWidth() - 140) / 2, yPos, 140, labelHeight);
    panel.reverseProbabilitySlider.setBounds((getWidth() - knobSize) / 2, yPos + labelHeight + 5, knobSize, knobSize);
}

//==============================================================================
// Main Editor Implementation
//==============================================================================
GrainsAudioProcessorEditor::GrainsAudioProcessorEditor (GrainsAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set up load sample button - Hypnos sleepy theme
    loadSampleButton.setButtonText("Load Sample");
    loadSampleButton.addListener(this);
    loadSampleButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffb4a5d8));  // Soft Lavender
    loadSampleButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff1a2332));  // Midnight Blue
    loadSampleButton.setClickingTogglesState(false);
    loadSampleButton.setEnabled(true);
    addAndMakeVisible(loadSampleButton);

    // Write to a log file for debugging
    juce::File logFile = juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("grains_debug.txt");
    logFile.appendText("Grains VST Plugin Initialized\n");

    // Setup main viewport for scrollable content
    mainViewport.setViewedComponent(&mainContentComponent, false);
    mainViewport.setScrollBarsShown(true, false);  // Auto-show vertical scrollbar
    addAndMakeVisible(mainViewport);

    // Setup section labels
    grainSourceSectionLabel.setText("GRAIN DESIGN", juce::dontSendNotification);
    grainSourceSectionLabel.setFont(juce::Font(juce::FontOptions("Arial", 11.0f, juce::Font::plain)));
    grainSourceSectionLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ba4d8).withAlpha(0.6f));
    grainSourceSectionLabel.setJustificationType(juce::Justification::left);
    mainContentComponent.addAndMakeVisible(grainSourceSectionLabel);

    outputSectionLabel.setText("OUTPUT", juce::dontSendNotification);
    outputSectionLabel.setFont(juce::Font(juce::FontOptions("Arial", 11.0f, juce::Font::plain)));
    outputSectionLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ba4d8).withAlpha(0.6f));
    outputSectionLabel.setJustificationType(juce::Justification::left);
    mainContentComponent.addAndMakeVisible(outputSectionLabel);

    // Unified visualization (waveform + grains + window overlay)
    unifiedVisualization.setGrainEngine(&audioProcessor.getGrainEngine());
    unifiedVisualization.setPositionParameter(dynamic_cast<juce::AudioParameterFloat*>(
        audioProcessor.getAPVTS().getParameter("position")));

    // Load sample if already loaded (for when UI reopens)
    if (audioProcessor.isSampleLoaded())
    {
        unifiedVisualization.setAudioBuffer(&audioProcessor.getSampleBuffer());
    }

    mainContentComponent.addAndMakeVisible(unifiedVisualization);

    // Set up all sliders and labels
    setupSlider(positionSlider, "position");
    setupLabel(positionLabel, "Position");
    positionAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "position", positionSlider));

    setupSlider(spraySlider, "spray");
    setupLabel(sprayLabel, "Spray");
    sprayAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "spray", spraySlider));

    setupSlider(grainSizeSlider, "grainSize");
    setupLabel(grainSizeLabel, "Size");
    grainSizeAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "grainSize", grainSizeSlider));

    setupSlider(densitySlider, "density");
    setupLabel(densityLabel, "Density");
    densityAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "density", densitySlider));

    setupSlider(pitchSlider, "pitch");
    setupLabel(pitchLabel, "Pitch");
    pitchAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "pitch", pitchSlider));

    setupSlider(panSlider, "pan");
    setupLabel(panLabel, "Pan");
    panAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "pan", panSlider));

    setupSlider(gainSlider, "gain");
    setupLabel(gainLabel, "Gain");
    gainAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "gain", gainSlider));

    // Phase 2 parameters
    setupSlider(panSpreadSlider, "panSpread");
    setupLabel(panSpreadLabel, "Pan Spread");
    panSpreadAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "panSpread", panSpreadSlider));

    // Reverse toggle (simple on/off)
    reverseToggle.setButtonText("REV");
    reverseToggle.setColour(juce::ToggleButton::textColourId, juce::Colour(0xfff5f1e8));
    reverseToggle.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xffb4a5d8));
    reverseToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xff4a4a52));
    mainContentComponent.addAndMakeVisible(reverseToggle);
    setupLabel(reverseLabel, "Reverse");
    reverseAttachment.reset(new ButtonAttachment(audioProcessor.getAPVTS(), "reverseEnabled", reverseToggle));

    // Grain Shape dropdown
    grainShapeCombo.addItem("Hann", 1);
    grainShapeCombo.addItem("Triangle", 2);
    grainShapeCombo.addItem("Trapezoid", 3);
    grainShapeCombo.addItem("Exponential", 4);
    grainShapeCombo.addItem("Gaussian", 5);
    grainShapeCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1a2332));
    grainShapeCombo.setColour(juce::ComboBox::textColourId, juce::Colour(0xfff5f1e8));
    grainShapeCombo.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff4a4a52));
    grainShapeCombo.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xffb4a5d8));
    mainContentComponent.addAndMakeVisible(grainShapeCombo);
    setupLabel(grainShapeLabel, "Shape");
    grainShapeAttachment.reset(new ComboBoxAttachment(audioProcessor.getAPVTS(), "grainShape", grainShapeCombo));

    // Phase 5 controls - Envelope
    setupSlider(envAttackSlider, "envAttack");
    setupLabel(envAttackLabel, "Attack");
    envAttackAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "envAttack", envAttackSlider));

    setupSlider(envDecaySlider, "envDecay");
    setupLabel(envDecayLabel, "Decay");
    envDecayAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "envDecay", envDecaySlider));

    setupSlider(envSustainSlider, "envSustain");
    setupLabel(envSustainLabel, "Sustain");
    envSustainAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "envSustain", envSustainSlider));

    setupSlider(envReleaseSlider, "envRelease");
    setupLabel(envReleaseLabel, "Release");
    envReleaseAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "envRelease", envReleaseSlider));

    // Advanced panel toggle button (styled with more contrast)
    advancedToggleButton.setButtonText("Advanced");
    advancedToggleButton.addListener(this);
    advancedToggleButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a4a52));
    advancedToggleButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffb4a5d8).withAlpha(0.3f));
    advancedToggleButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffb4a5d8));
    advancedToggleButton.setColour(juce::TextButton::textColourOnId, juce::Colour(0xfff5f1e8));
    advancedToggleButton.setClickingTogglesState(false);
    addAndMakeVisible(advancedToggleButton);

    // Create advanced panel overlay (initially hidden - default closed)
    advancedPanel = std::make_unique<AdvancedPanel>();
    addAndMakeVisible(advancedPanel.get());
    advancedPanel->setVisible(false);  // Start hidden

    // Connect close button to toggle handler
    advancedPanel->closeButton.addListener(this);

    // Setup LFO parameter attachments
    for (int i = 0; i < 4; ++i)
    {
        auto& controls = advancedPanel->lfoControls[i];
        auto& attachments = lfoAttachments[i];
        juce::String lfoNum = juce::String(i + 1);

        // Attach parameters to controls
        attachments.rateAttachment.reset(new SliderAttachment(
            audioProcessor.getAPVTS(), "lfo" + lfoNum + "Rate", controls.rateSlider));
        attachments.depthAttachment.reset(new SliderAttachment(
            audioProcessor.getAPVTS(), "lfo" + lfoNum + "Depth", controls.depthSlider));
        attachments.enabledAttachment.reset(new ButtonAttachment(
            audioProcessor.getAPVTS(), "lfo" + lfoNum + "Enabled", controls.enabledToggle));
        attachments.waveformAttachment.reset(new ComboBoxAttachment(
            audioProcessor.getAPVTS(), "lfo" + lfoNum + "Waveform", controls.waveformCombo));
        attachments.targetAttachment.reset(new ComboBoxAttachment(
            audioProcessor.getAPVTS(), "lfo" + lfoNum + "Target", controls.targetCombo));
    }

    // Setup advanced panel parameter attachments (Sound Design tab)
    reverseProbabilityAttachment.reset(new SliderAttachment(
        audioProcessor.getAPVTS(), "reverseProbability", advancedPanel->reverseProbabilitySlider));

    timeStretchAttachment.reset(new SliderAttachment(
        audioProcessor.getAPVTS(), "timeStretch", advancedPanel->timeStretchSlider));

    timeStretchToggleAttachment.reset(new ButtonAttachment(
        audioProcessor.getAPVTS(), "pitchTimeLock", advancedPanel->timeStretchToggle));

    // Setup time stretch toggle listener
    advancedPanel->timeStretchToggle.addListener(this);

    // Set window size - fixed, non-resizable
    setSize (1000, 600);
    setResizable(false, false);

    // Initialize time stretch slider state
    updateTimeStretchSliderState();

    // Start timer for visual feedback (30 Hz update rate)
    startTimer(33);  // ~30 FPS
}

GrainsAudioProcessorEditor::~GrainsAudioProcessorEditor()
{
    stopTimer();
    loadSampleButton.removeListener(this);
    advancedToggleButton.removeListener(this);
    advancedPanel->timeStretchToggle.removeListener(this);
    advancedPanel->closeButton.removeListener(this);
}

//==============================================================================
void GrainsAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Somnia sleepy nighttime background - gradient from midnight blue to darker
    juce::ColourGradient gradient(juce::Colour(0xff1a2332), 0, 0,
                                  juce::Colour(0xff0f1620), 0, static_cast<float>(getHeight()),
                                  false);
    g.setGradientFill(gradient);
    g.fillAll();

    // Title bar with subtle depth
    g.setColour(juce::Colour(0xff2d3e52).withAlpha(0.5f));
    g.fillRect(0, 0, getWidth(), 60);

    // Plugin title - Somnia
    g.setColour(juce::Colour(0xfff5f1e8));  // Cream
    g.setFont(juce::Font("Arial", 32.0f, juce::Font::plain));
    g.drawText("Somnia", 20, 10, 200, 40, juce::Justification::left);

    // Version with lavender accent
    g.setFont(juce::Font("Arial", 12.0f, juce::Font::plain));
    g.setColour(juce::Colour(0xffb4a5d8).withAlpha(0.7f));  // Soft Lavender
    g.drawText("v0.1 MVP", 20, 45, 200, 15, juce::Justification::left);

    // Draw drag-over overlay when dragging a file
    if (isDraggingFile)
    {
        g.setColour(juce::Colour(0xffb4a5d8).withAlpha(0.3f));
        g.fillAll();
        g.setColour(juce::Colour(0xffb4a5d8));
        g.drawRect(getLocalBounds().reduced(10), 3);
        g.setColour(juce::Colour(0xfff5f1e8));
        g.setFont(24.0f);
        g.drawText("Drop audio file here", getLocalBounds(), juce::Justification::centred);
    }
}

void GrainsAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    // Title area with Advanced and Load Sample buttons
    auto titleArea = area.removeFromTop(60);
    loadSampleButton.setBounds(titleArea.getRight() - 130, titleArea.getY() + 15, 110, 30);
    advancedToggleButton.setBounds(titleArea.getRight() - 260, titleArea.getY() + 15, 110, 30);

    // Main viewport for scrollable content
    mainViewport.setBounds(area);

    // Layout content inside the viewport
    auto contentArea = juce::Rectangle<int>(0, 0, area.getWidth(), 0);  // Height will be calculated

    // Waveform - taller, minimal gap
    contentArea.setHeight(2);
    auto visualArea = contentArea.withHeight(210).withY(contentArea.getBottom());
    unifiedVisualization.setBounds(visualArea.reduced(20, 0));
    contentArea.setHeight(contentArea.getHeight() + visualArea.getHeight());

    // Controls - 3 functional sections with consistent spacing
    const int margin = 20;
    const int knobSize = 60;  // Reduced by ~15% (was 70)
    const int labelHeight = 14;  // Tighter spacing
    const int sectionLabelHeight = 16;
    const int verticalGap = 26;  // Consistent vertical rhythm
    const int totalWidth = mainViewport.getWidth() - 2 * margin;

    contentArea.setHeight(contentArea.getHeight() + verticalGap);
    int yPos = contentArea.getBottom();

    // === SECTION 1: GRAIN SOURCE ===
    grainSourceSectionLabel.setBounds(margin, yPos, 200, sectionLabelHeight);
    yPos += sectionLabelHeight + 2;  // Tighter spacing

    int colWidth = totalWidth / 5;  // 5 controls
    const int comboWidth = 85;  // Skinnier dropdown

    // Position
    positionLabel.setBounds(margin, yPos, colWidth, labelHeight);
    positionSlider.setBounds(margin + (colWidth - knobSize) / 2, yPos + labelHeight, knobSize, knobSize);

    // Spray
    sprayLabel.setBounds(margin + colWidth, yPos, colWidth, labelHeight);
    spraySlider.setBounds(margin + colWidth + (colWidth - knobSize) / 2, yPos + labelHeight, knobSize, knobSize);

    // Grain Size
    grainSizeLabel.setBounds(margin + colWidth * 2, yPos, colWidth, labelHeight);
    grainSizeSlider.setBounds(margin + colWidth * 2 + (colWidth - knobSize) / 2, yPos + labelHeight, knobSize, knobSize);

    // Density
    densityLabel.setBounds(margin + colWidth * 3, yPos, colWidth, labelHeight);
    densitySlider.setBounds(margin + colWidth * 3 + (colWidth - knobSize) / 2, yPos + labelHeight, knobSize, knobSize);

    // Grain Shape - centered dropdown aligned with knob centers
    grainShapeLabel.setBounds(margin + colWidth * 4, yPos, colWidth, labelHeight);
    int dropdownY = yPos + labelHeight + (knobSize / 2) - 12;  // Center with knobs
    grainShapeCombo.setBounds(margin + colWidth * 4 + (colWidth - comboWidth) / 2, dropdownY, comboWidth, 24);

    // Reverse toggle - 6th item in GRAIN SOURCE row
    reverseLabel.setBounds(margin + colWidth * 5, yPos, colWidth, labelHeight);
    int toggleCenterY = yPos + labelHeight + (knobSize / 2) - 15;  // Center with knobs
    reverseToggle.setBounds(margin + colWidth * 5 + (colWidth - 60) / 2, toggleCenterY, 60, 30);

    int section1End = yPos + labelHeight + knobSize + 18 + verticalGap;
    contentArea.setHeight(section1End);
    yPos = section1End;

    // === SECTION 2: OUTPUT ===
    outputSectionLabel.setBounds(margin, yPos, 200, sectionLabelHeight);
    yPos += sectionLabelHeight + 2;  // Tighter spacing

    colWidth = totalWidth / 4;  // 4 controls evenly spaced

    // Pitch
    pitchLabel.setBounds(margin, yPos, colWidth, labelHeight);
    pitchSlider.setBounds(margin + (colWidth - knobSize) / 2, yPos + labelHeight, knobSize, knobSize);

    // Pan
    panLabel.setBounds(margin + colWidth, yPos, colWidth, labelHeight);
    panSlider.setBounds(margin + colWidth + (colWidth - knobSize) / 2, yPos + labelHeight, knobSize, knobSize);

    // Pan Spread
    panSpreadLabel.setBounds(margin + colWidth * 2, yPos, colWidth, labelHeight);
    panSpreadSlider.setBounds(margin + colWidth * 2 + (colWidth - knobSize) / 2, yPos + labelHeight, knobSize, knobSize);

    // Gain (rightmost - "volume tail")
    gainLabel.setBounds(margin + colWidth * 3, yPos, colWidth, labelHeight);
    gainSlider.setBounds(margin + colWidth * 3 + (colWidth - knobSize) / 2, yPos + labelHeight, knobSize, knobSize);

    // Move to next row for envelope controls
    yPos += labelHeight + knobSize + 18;

    // Envelope controls - second row in OUTPUT section
    // Attack
    envAttackLabel.setBounds(margin, yPos, colWidth, labelHeight);
    envAttackSlider.setBounds(margin + (colWidth - knobSize) / 2, yPos + labelHeight, knobSize, knobSize);

    // Decay
    envDecayLabel.setBounds(margin + colWidth, yPos, colWidth, labelHeight);
    envDecaySlider.setBounds(margin + colWidth + (colWidth - knobSize) / 2, yPos + labelHeight, knobSize, knobSize);

    // Sustain
    envSustainLabel.setBounds(margin + colWidth * 2, yPos, colWidth, labelHeight);
    envSustainSlider.setBounds(margin + colWidth * 2 + (colWidth - knobSize) / 2, yPos + labelHeight, knobSize, knobSize);

    // Release
    envReleaseLabel.setBounds(margin + colWidth * 3, yPos, colWidth, labelHeight);
    envReleaseSlider.setBounds(margin + colWidth * 3 + (colWidth - knobSize) / 2, yPos + labelHeight, knobSize, knobSize);

    int section2End = yPos + labelHeight + knobSize + 18;
    contentArea.setHeight(section2End + 20);  // +20 for bottom margin

    // Set the content component size
    mainContentComponent.setSize(mainViewport.getWidth(), contentArea.getHeight());

    // Set section border positions (between GRAIN SOURCE and OUTPUT sections)
    mainContentComponent.setSectionPositions(section1End - verticalGap / 2, section2End - verticalGap / 2);

    // Position advanced panel overlay (left side, full height, 600px wide)
    if (advancedPanel != nullptr)
    {
        advancedPanel->setBounds(0, 0, 600, getHeight());
    }
}

//==============================================================================
void GrainsAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    // Nothing needed here - drag tracking handled by sliderDragStarted/Ended
}

void GrainsAudioProcessorEditor::sliderDragStarted (juce::Slider* slider)
{
    // User started dragging - pause LFO visual updates for this slider
    slidersBeingDragged.insert(slider);
}

void GrainsAudioProcessorEditor::sliderDragEnded (juce::Slider* slider)
{
    // User finished dragging - resume LFO visual updates for this slider
    slidersBeingDragged.erase(slider);
}

void GrainsAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    juce::File logFile = juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("grains_debug.txt");
    logFile.appendText("Button clicked!\n");

    if (button == &loadSampleButton)
    {
        logFile.appendText("Load Sample button clicked!\n");
        DBG("Load Sample button clicked!");

        // Open file chooser (modern async API)
        auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

        auto fileChooser = std::make_shared<juce::FileChooser>(
            "Select an audio file to load...",
            juce::File::getSpecialLocation(juce::File::userHomeDirectory),
            "*.wav;*.aif;*.aiff;*.mp3;*.flac;*.ogg");

        logFile.appendText("About to launch file chooser...\n");

        fileChooser->launchAsync(chooserFlags, [this, fileChooser](const juce::FileChooser& chooser)
        {
            juce::File logFile2 = juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("grains_debug.txt");
            juce::File file = chooser.getResult();
            logFile2.appendText("File chooser returned: " + file.getFullPathName() + "\n");
            DBG("File chooser returned: " + file.getFullPathName());

            if (file.existsAsFile())
            {
                loadAudioFile(file);
            }
        });
    }
    else if (button == &advancedToggleButton)
    {
        toggleAdvancedPanel();
    }
    else if (button == &advancedPanel->closeButton)
    {
        toggleAdvancedPanel();
    }
    else if (button == &advancedPanel->timeStretchToggle)
    {
        updateTimeStretchSliderState();
    }
}

void GrainsAudioProcessorEditor::toggleAdvancedPanel()
{
    showAdvancedPanel = !showAdvancedPanel;
    advancedPanel->setVisible(showAdvancedPanel);
    advancedPanel->toFront(true);  // Bring to front when visible
}

void GrainsAudioProcessorEditor::updateTimeStretchSliderState()
{
    // When toggle is ON (locked/enabled), time stretch slider is disabled
    // When toggle is OFF (unlocked), time stretch slider is enabled
    bool isLocked = advancedPanel->timeStretchToggle.getToggleState();
    advancedPanel->timeStretchSlider.setEnabled(!isLocked);

    // Grey out when disabled
    advancedPanel->timeStretchSlider.setAlpha(isLocked ? 0.4f : 1.0f);
}

void GrainsAudioProcessorEditor::timerCallback()
{
    updateVisualFeedback();
}

void GrainsAudioProcessorEditor::updateVisualFeedback()
{
    // Get modulated parameter values from processor
    const auto& modValues = audioProcessor.getModulatedValues();

    // Helper lambda to update slider visual without triggering listener
    auto updateSliderVisual = [this](juce::Slider& slider, float modulatedValue)
    {
        // Only update if slider is not being dragged
        if (slidersBeingDragged.find(&slider) == slidersBeingDragged.end())
        {
            // Update slider visual position without sending notification
            slider.setValue(modulatedValue, juce::dontSendNotification);
        }
    };

    // Update all slider visuals with modulated values
    updateSliderVisual(positionSlider, modValues.position);
    updateSliderVisual(spraySlider, modValues.spray);
    updateSliderVisual(grainSizeSlider, modValues.grainSize);
    updateSliderVisual(densitySlider, modValues.density);
    updateSliderVisual(pitchSlider, modValues.pitch);
    updateSliderVisual(panSlider, modValues.pan);
    updateSliderVisual(panSpreadSlider, modValues.panSpread);
    // Note: reverse is now a toggle button, not a slider - no visual feedback needed
}

//==============================================================================
// Drag and drop support
bool GrainsAudioProcessorEditor::isInterestedInFileDrag (const juce::StringArray& files)
{
    juce::File logFile = juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("grains_debug.txt");
    logFile.appendText("isInterestedInFileDrag called with " + juce::String(files.size()) + " files\n");

    // Accept audio files
    for (const auto& file : files)
    {
        logFile.appendText("  Checking file: " + file + "\n");
        if (file.endsWithIgnoreCase(".wav") ||
            file.endsWithIgnoreCase(".aif") ||
            file.endsWithIgnoreCase(".aiff") ||
            file.endsWithIgnoreCase(".mp3") ||
            file.endsWithIgnoreCase(".flac") ||
            file.endsWithIgnoreCase(".ogg"))
        {
            logFile.appendText("  Accepted!\n");
            return true;
        }
    }
    logFile.appendText("  Rejected all files\n");
    return false;
}

void GrainsAudioProcessorEditor::filesDropped (const juce::StringArray& files, int x, int y)
{
    juce::ignoreUnused(x, y);
    juce::File logFile = juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("grains_debug.txt");
    logFile.appendText("filesDropped called!\n");

    isDraggingFile = false;
    repaint();

    if (files.size() > 0)
    {
        juce::File file(files[0]);
        logFile.appendText("File dropped: " + file.getFullPathName() + "\n");
        DBG("File dropped: " + file.getFullPathName());
        loadAudioFile(file);
    }
}

void GrainsAudioProcessorEditor::fileDragEnter (const juce::StringArray& files, int x, int y)
{
    juce::ignoreUnused(files, x, y);
    isDraggingFile = true;
    repaint();
}

void GrainsAudioProcessorEditor::fileDragExit (const juce::StringArray& files)
{
    juce::ignoreUnused(files);
    isDraggingFile = false;
    repaint();
}

//==============================================================================
void GrainsAudioProcessorEditor::loadAudioFile (const juce::File& file)
{
    juce::File logFile = juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("grains_debug.txt");
    logFile.appendText("loadAudioFile called: " + file.getFullPathName() + "\n");

    if (!file.existsAsFile())
    {
        logFile.appendText("ERROR: File does not exist!\n");
        DBG("File does not exist: " + file.getFullPathName());
        return;
    }

    logFile.appendText("File exists, loading...\n");
    DBG("Loading audio file: " + file.getFullPathName());
    audioProcessor.loadSampleFromFile(file);

    if (audioProcessor.isSampleLoaded())
    {
        // Update unified visualization with the loaded sample
        unifiedVisualization.setAudioBuffer(&audioProcessor.getSampleBuffer());

        logFile.appendText("SUCCESS: Sample loaded!\n");
        DBG("Sample loaded successfully!");
    }
    else
    {
        logFile.appendText("ERROR: Failed to load sample!\n");
        DBG("Failed to load sample!");
    }
}

//==============================================================================
void GrainsAudioProcessorEditor::setupSlider (juce::Slider& slider, const juce::String& paramID)
{
    // Somnia sleepy theme - rotary knobs (reduced size by ~15% for professional look)
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);  // Compact text box
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffb4a5d8));  // Soft Lavender
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff4a4a52));  // Warm Charcoal
    slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffd4a5a5));  // Dusty Rose accent
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xfff5f1e8));  // Cream
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff1a2332));  // Midnight Blue
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff4a4a52));  // Warm Charcoal

    slider.addListener(this);
    mainContentComponent.addAndMakeVisible(slider);
}

void GrainsAudioProcessorEditor::setupLabel (juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, juce::Colour(0xfff5f1e8).withAlpha(0.6f));  // Cream with 60% opacity
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font("Arial", 14.0f, juce::Font::plain));
    mainContentComponent.addAndMakeVisible(label);
}
