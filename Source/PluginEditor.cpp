#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// MainContentComponent Implementation
//==============================================================================
void GrainsAudioProcessorEditor::MainContentComponent::paint(juce::Graphics& g)
{
    // Draw rounded borders around the three sections
    if (!grainDesignBounds.isEmpty())
    {
        // Grain Design section - subtle border with rounded corners
        g.setColour(juce::Colour(0xff334155).withAlpha(0.3f));  // Slate border
        auto bounds = grainDesignBounds.reduced(4, 4).toFloat();
        g.drawRoundedRectangle(bounds, 12.0f, 1.5f);

        // Subtle inner glow
        g.setColour(juce::Colour(0xffa78bfa).withAlpha(0.05f));  // Purple tint for Grain Design
        g.fillRoundedRectangle(bounds, 12.0f);
    }

    if (!filtersBounds.isEmpty())
    {
        // Filters section - subtle border with rounded corners
        g.setColour(juce::Colour(0xff334155).withAlpha(0.3f));  // Slate border
        auto bounds = filtersBounds.reduced(4, 4).toFloat();
        g.drawRoundedRectangle(bounds, 12.0f, 1.5f);

        // Subtle inner glow
        g.setColour(juce::Colour(0xffa78bfa).withAlpha(0.05f));  // Purple tint for Filters
        g.fillRoundedRectangle(bounds, 12.0f);
    }

    if (!outputBounds.isEmpty())
    {
        // Output section - subtle border with rounded corners
        g.setColour(juce::Colour(0xff334155).withAlpha(0.3f));  // Slate border
        auto bounds = outputBounds.reduced(4, 4).toFloat();
        g.drawRoundedRectangle(bounds, 12.0f, 1.5f);

        // Subtle inner glow
        g.setColour(juce::Colour(0xfff472b6).withAlpha(0.05f));  // Pink tint for Output
        g.fillRoundedRectangle(bounds, 12.0f);
    }
}

void GrainsAudioProcessorEditor::MainContentComponent::setSectionPositions(int section1End, int section2End)
{
    section1EndY = section1End;
    section2EndY = section2End;
    repaint();
}

void GrainsAudioProcessorEditor::MainContentComponent::setColumnBounds(juce::Rectangle<int> grainDesign, juce::Rectangle<int> filters, juce::Rectangle<int> output)
{
    grainDesignBounds = grainDesign;
    filtersBounds = filters;
    outputBounds = output;
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

    // Setup LFO controls (styling only - added to content in ModulationTab constructor)
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

        controls.rateLabel.setText("LFO" + lfoNum + " Rate", juce::dontSendNotification);
        controls.rateLabel.setFont(juce::Font(juce::FontOptions("Arial", 12.0f, juce::Font::plain)));
        controls.rateLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ba4d8));
        controls.rateLabel.setJustificationType(juce::Justification::centred);

        // Depth slider
        controls.depthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        controls.depthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        controls.depthSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffb4a5d8));
        controls.depthSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff5f1e8));
        controls.depthSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff4a4a52));

        controls.depthLabel.setText("LFO" + lfoNum + " Depth", juce::dontSendNotification);
        controls.depthLabel.setFont(juce::Font(juce::FontOptions("Arial", 12.0f, juce::Font::plain)));
        controls.depthLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ba4d8));
        controls.depthLabel.setJustificationType(juce::Justification::centred);

        // Enabled toggle button
        controls.enabledToggle.setButtonText("ON");
        controls.enabledToggle.setColour(juce::ToggleButton::textColourId, juce::Colour(0xfff5f1e8));
        controls.enabledToggle.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xffb4a5d8));
        controls.enabledToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xff4a4a52));

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

        controls.waveformLabel.setText("Waveform", juce::dontSendNotification);
        controls.waveformLabel.setFont(juce::Font(juce::FontOptions("Arial", 12.0f, juce::Font::plain)));
        controls.waveformLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ba4d8));
        controls.waveformLabel.setJustificationType(juce::Justification::centred);

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

        controls.targetLabel.setText("Target", juce::dontSendNotification);
        controls.targetLabel.setFont(juce::Font(juce::FontOptions("Arial", 12.0f, juce::Font::plain)));
        controls.targetLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ba4d8));
        controls.targetLabel.setJustificationType(juce::Justification::centred);
    }

    // Setup Sound Design controls (styling only - added to content in SoundDesignTab constructor)
    // Time Stretch slider
    timeStretchSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    timeStretchSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    timeStretchSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffb4a5d8));
    timeStretchSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff5f1e8));
    timeStretchSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff4a4a52));
    timeStretchSlider.setTooltip("Playback speed multiplier (0.25x-4x). Independent from pitch when unlocked. 1x = normal speed.");

    timeStretchLabel.setText("Time", juce::dontSendNotification);
    timeStretchLabel.setFont(juce::Font(juce::FontOptions("Arial", 13.0f, juce::Font::plain)));
    timeStretchLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ba4d8));
    timeStretchLabel.setJustificationType(juce::Justification::centred);

    // Time Stretch toggle (pitch/time lock)
    timeStretchToggle.setButtonText("LOCK");
    timeStretchToggle.setColour(juce::ToggleButton::textColourId, juce::Colour(0xfff5f1e8));
    timeStretchToggle.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xffb4a5d8));
    timeStretchToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xff4a4a52));
    timeStretchToggle.setTooltip("When enabled: pitch and time are locked together (classic behavior). Disabled = independent control.");

    timeStretchToggleLabel.setText("Pitch/Time Lock", juce::dontSendNotification);
    timeStretchToggleLabel.setFont(juce::Font(juce::FontOptions("Arial", 13.0f, juce::Font::plain)));
    timeStretchToggleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ba4d8));
    timeStretchToggleLabel.setJustificationType(juce::Justification::centred);

    // Reverse Probability slider
    reverseProbabilitySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    reverseProbabilitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    reverseProbabilitySlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffb4a5d8));
    reverseProbabilitySlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff5f1e8));
    reverseProbabilitySlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff4a4a52));
    reverseProbabilitySlider.setTooltip("Probability that grains play backwards (0-100%). 0% = all forward, 100% = all reverse, 50% = random mix.");

    reverseProbabilityLabel.setText("Reverse Probability", juce::dontSendNotification);
    reverseProbabilityLabel.setFont(juce::Font(juce::FontOptions("Arial", 13.0f, juce::Font::plain)));
    reverseProbabilityLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ba4d8));
    reverseProbabilityLabel.setJustificationType(juce::Justification::centred);

    // Octave Spread slider
    octaveSpreadSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    octaveSpreadSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    octaveSpreadSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffb4a5d8));
    octaveSpreadSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff5f1e8));
    octaveSpreadSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff4a4a52));
    octaveSpreadSlider.setTooltip("Octave shift range (0-2 octaves). Grains randomly shift ±spread. Creates layered harmonic textures.");

    octaveSpreadLabel.setText("Octave Spread", juce::dontSendNotification);
    octaveSpreadLabel.setFont(juce::Font(juce::FontOptions("Arial", 13.0f, juce::Font::plain)));
    octaveSpreadLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ba4d8));
    octaveSpreadLabel.setJustificationType(juce::Justification::centred);

    // Octave Probability slider
    octaveProbabilitySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    octaveProbabilitySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    octaveProbabilitySlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffb4a5d8));
    octaveProbabilitySlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff5f1e8));
    octaveProbabilitySlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff4a4a52));
    octaveProbabilitySlider.setTooltip("Chance of octave shifting per grain (0-100%). Higher = more grains affected by octave spread.");

    octaveProbabilityLabel.setText("Octave Probability", juce::dontSendNotification);
    octaveProbabilityLabel.setFont(juce::Font(juce::FontOptions("Arial", 13.0f, juce::Font::plain)));
    octaveProbabilityLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ba4d8));
    octaveProbabilityLabel.setJustificationType(juce::Justification::centred);
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
GrainsAudioProcessorEditor::AdvancedPanel::ModulationTab::ModulationTab(AdvancedPanel& parent)
    : panel(parent)
{
    // Setup viewport for scrolling
    viewport.setViewedComponent(&contentComponent, false);
    viewport.setScrollBarsShown(true, false);  // Vertical scrollbar, no horizontal
    addAndMakeVisible(viewport);

    // Add all LFO controls to the content component instead of directly to the tab
    for (int i = 0; i < 4; ++i)
    {
        auto& controls = panel.lfoControls[i];
        contentComponent.addAndMakeVisible(controls.rateSlider);
        contentComponent.addAndMakeVisible(controls.rateLabel);
        contentComponent.addAndMakeVisible(controls.depthSlider);
        contentComponent.addAndMakeVisible(controls.depthLabel);
        contentComponent.addAndMakeVisible(controls.enabledToggle);
        contentComponent.addAndMakeVisible(controls.waveformCombo);
        contentComponent.addAndMakeVisible(controls.waveformLabel);
        contentComponent.addAndMakeVisible(controls.targetCombo);
        contentComponent.addAndMakeVisible(controls.targetLabel);
    }
}

void GrainsAudioProcessorEditor::AdvancedPanel::ModulationTab::resized()
{
    // Prevent recursive resize calls
    if (isResizing)
        return;

    juce::ScopedValueSetter<bool> resizeGuard(isResizing, true);

    // Fill the tab with the viewport
    viewport.setBounds(getLocalBounds());

    const int margin = 20;
    const int knobSize = 70;  // Increased from 50px for better visibility
    const int comboWidth = 230;
    const int labelHeight = 14;
    const int lfoBlockHeight = 200;  // Increased to accommodate larger knobs
    // Use full width - don't subtract scrollbar thickness (let viewport handle it)
    const int contentWidth = getWidth();
    const int columnWidth = (contentWidth - 3 * margin) / 2;

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

    // Calculate total content height needed
    int totalHeight = yPos + 2 * (lfoBlockHeight + 15) + margin;
    contentComponent.setSize(contentWidth, totalHeight);
}

//==============================================================================
// SoundDesignTab Implementation
//==============================================================================
GrainsAudioProcessorEditor::AdvancedPanel::SoundDesignTab::SoundDesignTab(AdvancedPanel& parent)
    : panel(parent)
{
    // Setup viewport for scrolling
    viewport.setViewedComponent(&contentComponent, false);
    viewport.setScrollBarsShown(true, false);  // Vertical scrollbar, no horizontal
    addAndMakeVisible(viewport);

    // Add all sound design controls to the content component
    contentComponent.addAndMakeVisible(panel.timeStretchSlider);
    contentComponent.addAndMakeVisible(panel.timeStretchLabel);
    contentComponent.addAndMakeVisible(panel.timeStretchToggle);
    contentComponent.addAndMakeVisible(panel.timeStretchToggleLabel);
    contentComponent.addAndMakeVisible(panel.reverseProbabilitySlider);
    contentComponent.addAndMakeVisible(panel.reverseProbabilityLabel);
    contentComponent.addAndMakeVisible(panel.octaveSpreadSlider);
    contentComponent.addAndMakeVisible(panel.octaveSpreadLabel);
    contentComponent.addAndMakeVisible(panel.octaveProbabilitySlider);
    contentComponent.addAndMakeVisible(panel.octaveProbabilityLabel);
    contentComponent.addAndMakeVisible(panel.grainShapeCombo);
    contentComponent.addAndMakeVisible(panel.grainShapeLabel);
}

void GrainsAudioProcessorEditor::AdvancedPanel::SoundDesignTab::resized()
{
    // Prevent recursive resize calls
    if (isResizing)
        return;

    juce::ScopedValueSetter<bool> resizeGuard(isResizing, true);

    // Fill the tab with the viewport
    viewport.setBounds(getLocalBounds());

    const int margin = 30;
    const int knobSize = 80;
    const int labelHeight = 16;
    const int rowSpacing = 110;  // Spacing between rows
    // Use full width - don't subtract scrollbar thickness (let viewport handle it)
    const int contentWidth = getWidth();

    int yPos = margin + 20;

    // Calculate column positions (2-column layout)
    int totalWidth = contentWidth - (2 * margin);
    int col1X = margin + (totalWidth / 4) - (knobSize / 2);  // Center of left quarter
    int col2X = margin + (3 * totalWidth / 4) - (knobSize / 2);  // Center of right quarter

    // Row 1: Time Stretch (left) | Reverse Probability (right)
    panel.timeStretchLabel.setBounds(col1X - 10, yPos, 120, labelHeight);
    panel.timeStretchSlider.setBounds(col1X, yPos + labelHeight + 5, knobSize, knobSize);

    panel.reverseProbabilityLabel.setBounds(col2X - 20, yPos, 140, labelHeight);
    panel.reverseProbabilitySlider.setBounds(col2X, yPos + labelHeight + 5, knobSize, knobSize);

    yPos += labelHeight + knobSize + 15;

    // Row 2: Time Stretch Toggle (left, below time stretch)
    panel.timeStretchToggleLabel.setBounds(col1X - 10, yPos, 120, labelHeight);
    panel.timeStretchToggle.setBounds(col1X + 5, yPos + labelHeight + 5, 70, 30);

    yPos += rowSpacing;

    // Row 3: Octave Spread (left) | Octave Probability (right)
    panel.octaveSpreadLabel.setBounds(col1X - 10, yPos, 120, labelHeight);
    panel.octaveSpreadSlider.setBounds(col1X, yPos + labelHeight + 5, knobSize, knobSize);

    panel.octaveProbabilityLabel.setBounds(col2X - 20, yPos, 140, labelHeight);
    panel.octaveProbabilitySlider.setBounds(col2X, yPos + labelHeight + 5, knobSize, knobSize);

    yPos += rowSpacing;

    // Row 4: Grain Shape (centered dropdown)
    int centerX = contentWidth / 2;
    panel.grainShapeLabel.setBounds(centerX - 60, yPos, 120, labelHeight);
    panel.grainShapeCombo.setBounds(centerX - 100, yPos + labelHeight + 5, 200, 32);

    // Calculate total content height needed
    int totalHeight = yPos + labelHeight + 32 + margin;  // label + combo + margin
    contentComponent.setSize(contentWidth, totalHeight);
}

//==============================================================================
// Main Editor Implementation
//==============================================================================
GrainsAudioProcessorEditor::GrainsAudioProcessorEditor (GrainsAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), tooltipWindow(this, 1500)
{
    // Set up load sample button - Hypnos sleepy theme
    loadSampleButton.setButtonText("Load Sample");
    loadSampleButton.addListener(this);
    loadSampleButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffb4a5d8));  // Soft Lavender
    loadSampleButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff1a2332));  // Midnight Blue
    loadSampleButton.setClickingTogglesState(false);
    loadSampleButton.setEnabled(true);
    loadSampleButton.setTooltip("Load an audio file (.wav, .aif, .mp3) to use as the grain source.");
    addAndMakeVisible(loadSampleButton);

    // Write to a log file for debugging
    juce::File logFile = juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getChildFile("grains_debug.txt");
    logFile.appendText("Grains VST Plugin Initialized\n");

    // Setup main viewport for scrollable content
    mainViewport.setViewedComponent(&mainContentComponent, false);
    mainViewport.setScrollBarsShown(true, false, true, false);  // Always show vertical scrollbar, never horizontal
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
    positionSlider.setTooltip("Playback position in the sample (0-100%). Shows as a window overlay on the waveform.");
    positionAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "position", positionSlider));

    setupSlider(spraySlider, "spray");
    setupLabel(sprayLabel, "Spray");
    spraySlider.setTooltip("Randomizes grain position around the main Position value. Higher values = more variation.");
    sprayAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "spray", spraySlider));

    setupSlider(grainSizeSlider, "grainSize");
    setupLabel(grainSizeLabel, "Size");
    grainSizeSlider.setTooltip("Duration of each grain in milliseconds (5-500ms). Shorter = more granular, longer = smoother.");
    grainSizeAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "grainSize", grainSizeSlider));

    setupSlider(densitySlider, "density");
    setupLabel(densityLabel, "Density");
    densitySlider.setTooltip("Number of grains per second (1-100). Higher values create denser, more continuous textures.");
    densityAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "density", densitySlider));

    // Output section sliders (pink color #f472b6)
    setupSlider(pitchSlider, "pitch");
    setupLabel(pitchLabel, "Pitch");
    pitchSlider.setTooltip("Pitch shift in semitones (-24 to +24). Follows MIDI note pitch automatically.");
    pitchSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xfff472b6));  // Pink
    pitchSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff9a8d4));  // Light pink accent
    pitchAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "pitch", pitchSlider));

    setupSlider(panSlider, "pan");
    setupLabel(panLabel, "Pan");
    panSlider.setTooltip("Stereo position of grains. -100 = left, 0 = center, +100 = right.");
    panSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xfff472b6));  // Pink
    panSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff9a8d4));  // Light pink accent
    panAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "pan", panSlider));

    setupSlider(gainSlider, "gain");
    setupLabel(gainLabel, "Gain");
    gainSlider.setTooltip("Output volume in decibels (-60 to +12 dB). Use MIDI velocity for per-note dynamics.");
    gainSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xfff472b6));  // Pink
    gainSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff9a8d4));  // Light pink accent
    gainAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "gain", gainSlider));

    // Phase 2 parameters
    setupSlider(panSpreadSlider, "panSpread");
    setupLabel(panSpreadLabel, "Pan Spread");
    panSpreadSlider.setTooltip("Randomizes pan position for each grain (0-100%). Creates wider stereo field.");
    panSpreadSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xfff472b6));  // Pink
    panSpreadSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff9a8d4));  // Light pink accent
    panSpreadAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "panSpread", panSpreadSlider));

    // Reverse toggle (simple on/off)
    reverseToggle.setButtonText("REV");
    reverseToggle.setColour(juce::ToggleButton::textColourId, juce::Colour(0xfff5f1e8));
    reverseToggle.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xffb4a5d8));
    reverseToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xff4a4a52));
    mainContentComponent.addAndMakeVisible(reverseToggle);
    setupLabel(reverseLabel, "Reverse");
    reverseAttachment.reset(new ButtonAttachment(audioProcessor.getAPVTS(), "reverseEnabled", reverseToggle));

    // Phase 5 controls - Envelope
    // Interactive ADSR Envelope (replaces separate knobs + visualizer)
    interactiveADSR.setAttackParameter(dynamic_cast<juce::AudioParameterFloat*>(audioProcessor.getAPVTS().getParameter("envAttack")));
    interactiveADSR.setDecayParameter(dynamic_cast<juce::AudioParameterFloat*>(audioProcessor.getAPVTS().getParameter("envDecay")));
    interactiveADSR.setSustainParameter(dynamic_cast<juce::AudioParameterFloat*>(audioProcessor.getAPVTS().getParameter("envSustain")));
    interactiveADSR.setReleaseParameter(dynamic_cast<juce::AudioParameterFloat*>(audioProcessor.getAPVTS().getParameter("envRelease")));
    mainContentComponent.addAndMakeVisible(interactiveADSR);

    // Filter section
    filterSectionLabel.setText("FILTER", juce::dontSendNotification);
    filterSectionLabel.setFont(juce::Font(juce::FontOptions("Arial", 11.0f, juce::Font::plain)));
    filterSectionLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9ba4d8).withAlpha(0.6f));
    filterSectionLabel.setJustificationType(juce::Justification::left);
    mainContentComponent.addAndMakeVisible(filterSectionLabel);

    // Low-pass cutoff
    setupSlider(lpFilterCutoffSlider, "lpFilterCutoff");
    setupLabel(lpFilterCutoffLabel, "LP Cutoff");
    lpFilterCutoffSlider.setTooltip("Low-pass cutoff frequency (20Hz - 20kHz). Frequencies above this are attenuated.");
    lpFilterCutoffAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "lpFilterCutoff", lpFilterCutoffSlider));

    // Low-pass resonance
    setupSlider(lpFilterResonanceSlider, "lpFilterResonance");
    setupLabel(lpFilterResonanceLabel, "LP Res");
    lpFilterResonanceSlider.setTooltip("Low-pass resonance (Q factor). Emphasizes frequencies near the cutoff. Higher = sharper peak.");
    lpFilterResonanceAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "lpFilterResonance", lpFilterResonanceSlider));

    // High-pass cutoff
    setupSlider(hpFilterCutoffSlider, "hpFilterCutoff");
    setupLabel(hpFilterCutoffLabel, "HP Cutoff");
    hpFilterCutoffSlider.setTooltip("High-pass cutoff frequency (20Hz - 20kHz). Frequencies below this are attenuated.");
    hpFilterCutoffAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "hpFilterCutoff", hpFilterCutoffSlider));

    // High-pass resonance
    setupSlider(hpFilterResonanceSlider, "hpFilterResonance");
    setupLabel(hpFilterResonanceLabel, "HP Res");
    hpFilterResonanceSlider.setTooltip("High-pass resonance (Q factor). Emphasizes frequencies near the cutoff. Higher = sharper peak.");
    hpFilterResonanceAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "hpFilterResonance", hpFilterResonanceSlider));

    // Advanced panel toggle button (styled with more contrast)
    advancedToggleButton.setButtonText("Advanced");
    advancedToggleButton.setTooltip("Open advanced panel for LFO modulation and sound design parameters.");
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

    octaveSpreadAttachment.reset(new SliderAttachment(
        audioProcessor.getAPVTS(), "octaveSpread", advancedPanel->octaveSpreadSlider));

    octaveProbabilityAttachment.reset(new SliderAttachment(
        audioProcessor.getAPVTS(), "octaveProbability", advancedPanel->octaveProbabilitySlider));

    timeStretchAttachment.reset(new SliderAttachment(
        audioProcessor.getAPVTS(), "timeStretch", advancedPanel->timeStretchSlider));

    // Grain Shape dropdown (styling only - added to content in SoundDesignTab constructor)
    advancedPanel->grainShapeCombo.addItem("Hann", 1);
    advancedPanel->grainShapeCombo.addItem("Triangle", 2);
    advancedPanel->grainShapeCombo.addItem("Trapezoid", 3);
    advancedPanel->grainShapeCombo.addItem("Exponential", 4);
    advancedPanel->grainShapeCombo.addItem("Gaussian", 5);
    advancedPanel->grainShapeCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1a2332));
    advancedPanel->grainShapeCombo.setColour(juce::ComboBox::textColourId, juce::Colour(0xfff5f1e8));
    advancedPanel->grainShapeCombo.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff4a4a52));
    advancedPanel->grainShapeCombo.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xffb4a5d8));
    advancedPanel->grainShapeCombo.setTooltip("Envelope shape for each grain. Hann = smooth, Triangle = sharper, Exponential = natural decay.");

    advancedPanel->grainShapeLabel.setText("Shape", juce::dontSendNotification);
    advancedPanel->grainShapeLabel.setColour(juce::Label::textColourId, juce::Colour(0xfff5f1e8).withAlpha(0.6f));
    advancedPanel->grainShapeLabel.setJustificationType(juce::Justification::centred);
    advancedPanel->grainShapeLabel.setFont(juce::Font("Arial", 14.0f, juce::Font::plain));

    grainShapeAttachment.reset(new ComboBoxAttachment(audioProcessor.getAPVTS(), "grainShape", advancedPanel->grainShapeCombo));

    timeStretchToggleAttachment.reset(new ButtonAttachment(
        audioProcessor.getAPVTS(), "pitchTimeLock", advancedPanel->timeStretchToggle));

    // Setup time stretch toggle listener
    advancedPanel->timeStretchToggle.addListener(this);

    // Set window size - resizable with constraints
    setSize (1000, 500);  // Default size
    setResizable(true, true);  // Allow resizing with constraints

    // Set minimum and maximum size constraints
    // Minimum: 800x400 (keeps UI usable)
    // Maximum: 2400x1600 (reasonable for large displays)
    setResizeLimits(800, 400, 2400, 1600);

    // Add corner resizer component for visual feedback
    getConstrainer()->setMinimumSize(800, 400);
    getConstrainer()->setMaximumSize(2400, 1600);

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
    g.drawText("v1.0 BETA", 20, 45, 200, 15, juce::Justification::left);

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
    // Prevent recursive resize calls that cause jittering
    if (isResizing)
        return;

    auto area = getLocalBounds();

    // Only proceed if size has actually changed (prevents jitter from redundant resizes)
    if (area == lastBounds)
        return;

    lastBounds = area;
    juce::ScopedValueSetter<bool> resizeGuard(isResizing, true);

    // Title area with Advanced and Load Sample buttons
    auto titleArea = area.removeFromTop(60);
    loadSampleButton.setBounds(titleArea.getRight() - 130, titleArea.getY() + 15, 110, 30);
    advancedToggleButton.setBounds(titleArea.getRight() - 260, titleArea.getY() + 15, 110, 30);

    // Main viewport for scrollable content
    mainViewport.setBounds(area);

    // Layout content inside the viewport
    // Account for scrollbar width to prevent content from overlapping with it
    const int scrollBarWidth = mainViewport.getScrollBarThickness();
    const int stableWidth = area.getWidth() - scrollBarWidth;
    auto contentArea = juce::Rectangle<int>(0, 0, stableWidth, 0);  // Height will be calculated

    // Waveform display at top
    contentArea.setHeight(2);
    auto visualArea = contentArea.withHeight(170).withY(contentArea.getBottom());  // Reduced from 210 to 170
    unifiedVisualization.setBounds(visualArea.reduced(20, 0));
    contentArea.setHeight(contentArea.getHeight() + visualArea.getHeight());

    // Constants
    const int margin = 20;
    const int gap = 16;  // Gap between columns
    const int sectionPadding = 16;
    const int knobSize = 64;  // Medium knobs (matching React design)
    const int labelHeight = 12;
    const int sectionLabelHeight = 11;
    const int totalWidth = stableWidth - 2 * margin;
    const int columnWidth = (totalWidth - gap) / 2;  // Two equal columns

    contentArea.setHeight(contentArea.getHeight() + 16);
    int yPos = contentArea.getBottom();

    // === 2-COLUMN LAYOUT ===
    int leftX = margin;
    int rightX = margin + columnWidth + gap;

    // LEFT COLUMN: GRAIN DESIGN
    grainSourceSectionLabel.setBounds(leftX, yPos, columnWidth, sectionLabelHeight);
    int leftYPos = yPos + sectionLabelHeight + 8;

    // 1 row of 4 knobs in Grain Design (matching Output section layout)
    int grainKnobWidth = columnWidth / 4;

    // Row 1: Position | Spray | Size | Density
    positionLabel.setBounds(leftX, leftYPos, grainKnobWidth, labelHeight);
    positionSlider.setBounds(leftX + (grainKnobWidth - knobSize) / 2, leftYPos + labelHeight, knobSize, knobSize);

    sprayLabel.setBounds(leftX + grainKnobWidth, leftYPos, grainKnobWidth, labelHeight);
    spraySlider.setBounds(leftX + grainKnobWidth + (grainKnobWidth - knobSize) / 2, leftYPos + labelHeight, knobSize, knobSize);

    grainSizeLabel.setBounds(leftX + grainKnobWidth * 2, leftYPos, grainKnobWidth, labelHeight);
    grainSizeSlider.setBounds(leftX + grainKnobWidth * 2 + (grainKnobWidth - knobSize) / 2, leftYPos + labelHeight, knobSize, knobSize);

    densityLabel.setBounds(leftX + grainKnobWidth * 3, leftYPos, grainKnobWidth, labelHeight);
    densitySlider.setBounds(leftX + grainKnobWidth * 3 + (grainKnobWidth - knobSize) / 2, leftYPos + labelHeight, knobSize, knobSize);

    leftYPos += labelHeight + knobSize + 16;

    // Save grain design end position for border calculation
    int grainDesignEndY = leftYPos;

    // Add gap between sections
    leftYPos += 8;

    // FILTERS SECTION (below Grain Design)
    int filterStartY = leftYPos;  // Save where filter section starts (at the label)
    filterSectionLabel.setBounds(leftX, leftYPos, columnWidth, sectionLabelHeight);
    leftYPos += sectionLabelHeight + 8;

    // 1 row of 4 knobs: LP Cutoff | LP Res | HP Cutoff | HP Res
    int filterKnobWidth = columnWidth / 4;

    // LP Cutoff
    lpFilterCutoffLabel.setBounds(leftX, leftYPos, filterKnobWidth, labelHeight);
    lpFilterCutoffSlider.setBounds(leftX + (filterKnobWidth - knobSize) / 2, leftYPos + labelHeight, knobSize, knobSize);

    // LP Resonance
    lpFilterResonanceLabel.setBounds(leftX + filterKnobWidth, leftYPos, filterKnobWidth, labelHeight);
    lpFilterResonanceSlider.setBounds(leftX + filterKnobWidth + (filterKnobWidth - knobSize) / 2, leftYPos + labelHeight, knobSize, knobSize);

    // HP Cutoff
    hpFilterCutoffLabel.setBounds(leftX + filterKnobWidth * 2, leftYPos, filterKnobWidth, labelHeight);
    hpFilterCutoffSlider.setBounds(leftX + filterKnobWidth * 2 + (filterKnobWidth - knobSize) / 2, leftYPos + labelHeight, knobSize, knobSize);

    // HP Resonance
    hpFilterResonanceLabel.setBounds(leftX + filterKnobWidth * 3, leftYPos, filterKnobWidth, labelHeight);
    hpFilterResonanceSlider.setBounds(leftX + filterKnobWidth * 3 + (filterKnobWidth - knobSize) / 2, leftYPos + labelHeight, knobSize, knobSize);

    leftYPos += labelHeight + knobSize + 16;

    // Store filter section end position before matching to right column
    int filterSectionEndY = leftYPos;

    // RIGHT COLUMN: OUTPUT
    outputSectionLabel.setBounds(rightX, yPos, columnWidth, sectionLabelHeight);
    int rightYPos = yPos + sectionLabelHeight + 8;

    // 4 knobs in a row: Pitch | Pan | Pan Spread | Gain
    int outputKnobWidth = columnWidth / 4;

    pitchLabel.setBounds(rightX, rightYPos, outputKnobWidth, labelHeight);
    pitchSlider.setBounds(rightX + (outputKnobWidth - knobSize) / 2, rightYPos + labelHeight, knobSize, knobSize);

    panLabel.setBounds(rightX + outputKnobWidth, rightYPos, outputKnobWidth, labelHeight);
    panSlider.setBounds(rightX + outputKnobWidth + (outputKnobWidth - knobSize) / 2, rightYPos + labelHeight, knobSize, knobSize);

    panSpreadLabel.setBounds(rightX + outputKnobWidth * 2, rightYPos, outputKnobWidth, labelHeight);
    panSpreadSlider.setBounds(rightX + outputKnobWidth * 2 + (outputKnobWidth - knobSize) / 2, rightYPos + labelHeight, knobSize, knobSize);

    gainLabel.setBounds(rightX + outputKnobWidth * 3, rightYPos, outputKnobWidth, labelHeight);
    gainSlider.setBounds(rightX + outputKnobWidth * 3 + (outputKnobWidth - knobSize) / 2, rightYPos + labelHeight, knobSize, knobSize);

    rightYPos += labelHeight + knobSize + 16;

    // Interactive ADSR envelope (replaces separate ADSR knobs + visualizer)
    // Calculate ADSR height to match left column height (grain design + filters)
    // leftYPos is currently at the end of filters section
    // rightYPos is at the end of output knobs
    // ADSR should fill the space to make both columns equal height
    int adsrHeight = filterSectionEndY - rightYPos - 16;  // Match filter end position, minus gap
    interactiveADSR.setBounds(rightX, rightYPos, columnWidth, adsrHeight);

    rightYPos += adsrHeight + 16;

    // Extend filter section to match output section height
    leftYPos = rightYPos;

    // Calculate total height (both columns now match)
    int totalHeight = rightYPos;
    contentArea.setHeight(totalHeight + 10);  // +10 for minimal bottom margin

    // Set the content component bounds explicitly to prevent viewport feedback
    // Use setBounds instead of setSize to avoid triggering additional resize events
    mainContentComponent.setBounds(0, 0, stableWidth, contentArea.getHeight());

    // Clear section positions (no dividers in new design)
    mainContentComponent.setSectionPositions(0, 0);

    // Set section bounds for rounded borders
    // Add padding around the content of each section
    const int borderPadding = 8;

    // Grain Design section (top left)
    juce::Rectangle<int> grainDesignSection(leftX - borderPadding, yPos - borderPadding,
                                             columnWidth + borderPadding * 2, grainDesignEndY - yPos + borderPadding);

    // Filters section (bottom left, extends to match output section bottom)
    // Start at filter label position (with padding above) and extend to match output section bottom
    int filterSectionHeight = (rightYPos + borderPadding) - (filterStartY - borderPadding);
    juce::Rectangle<int> filtersSection(leftX - borderPadding, filterStartY - borderPadding,
                                         columnWidth + borderPadding * 2, filterSectionHeight);

    // Output section (right, full height)
    juce::Rectangle<int> outputSection(rightX - borderPadding, yPos - borderPadding,
                                        columnWidth + borderPadding * 2, rightYPos - yPos + borderPadding * 2);

    mainContentComponent.setColumnBounds(grainDesignSection, filtersSection, outputSection);

    // Position advanced panel overlay (left side, full height, 600px wide)
    // Only resize if visible to reduce unnecessary layout calculations
    if (advancedPanel != nullptr && advancedPanel->isVisible())
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
    updateModulationIndicators();
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

void GrainsAudioProcessorEditor::updateModulationIndicators()
{
    // Check which parameters are being modulated by any of the 4 LFOs
    // LFO Targets: 0=Position, 1=Spray, 2=Size, 3=Density, 4=Pitch, 5=Pan, 6=PanSpread, 7=Reverse, 8=None

    bool positionModulated = false;
    bool sprayModulated = false;
    bool sizeModulated = false;
    bool densityModulated = false;
    bool pitchModulated = false;
    bool panModulated = false;
    bool panSpreadModulated = false;

    // Check all 4 LFOs
    for (int i = 0; i < 4; ++i)
    {
        juce::String lfoNum = juce::String(i + 1);

        // Check if LFO is enabled
        bool lfoEnabled = audioProcessor.getAPVTS().getRawParameterValue("lfo" + lfoNum + "Enabled")->load() > 0.5f;
        if (!lfoEnabled)
            continue;

        // Check if LFO has depth > 0
        float lfoDepth = audioProcessor.getAPVTS().getRawParameterValue("lfo" + lfoNum + "Depth")->load();
        if (lfoDepth <= 0.01f)  // Essentially off
            continue;

        // Get LFO target
        int lfoTarget = static_cast<int>(audioProcessor.getAPVTS().getRawParameterValue("lfo" + lfoNum + "Target")->load());

        // Mark corresponding parameter as modulated
        switch (lfoTarget)
        {
            case 0: positionModulated = true; break;
            case 1: sprayModulated = true; break;
            case 2: sizeModulated = true; break;
            case 3: densityModulated = true; break;
            case 4: pitchModulated = true; break;
            case 5: panModulated = true; break;
            case 6: panSpreadModulated = true; break;
            case 7: /* reverse - no slider for this in main UI */ break;
            default: break;  // None or invalid
        }
    }

    // Update slider modulation states
    positionSlider.setModulated(positionModulated);
    spraySlider.setModulated(sprayModulated);
    grainSizeSlider.setModulated(sizeModulated);
    densitySlider.setModulated(densityModulated);
    pitchSlider.setModulated(pitchModulated);
    panSlider.setModulated(panModulated);
    panSpreadSlider.setModulated(panSpreadModulated);

    // Update pulse animation
    positionSlider.updatePulse();
    spraySlider.updatePulse();
    grainSizeSlider.updatePulse();
    densitySlider.updatePulse();
    pitchSlider.updatePulse();
    panSlider.updatePulse();
    panSpreadSlider.updatePulse();
}
