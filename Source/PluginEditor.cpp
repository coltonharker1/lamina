#include "PluginProcessor.h"
#include "PluginEditor.h"

// Color constants and font helpers are now sourced from sonorous-kit. The local
// names here are kept (as constexpr forwarders / thin wrappers) so the rest of
// this file's call sites don't need to change.
namespace
{
constexpr juce::uint32 paper    = sonorous::palette::paper;
constexpr juce::uint32 ink      = sonorous::palette::ink;
constexpr juce::uint32 quietInk = sonorous::palette::quietInk;
constexpr juce::uint32 faintInk = sonorous::palette::faintInk;

juce::FontOptions monoFont(juce::Typeface::Ptr typeface, float height)
{
    return juce::FontOptions(typeface).withHeight(height);
}

juce::FontOptions serifFont(juce::Typeface::Ptr typeface, float height)
{
    return juce::FontOptions(typeface).withHeight(height);
}
}

// MonoPrintLookAndFeel::* implementations were moved into sonorous_visual's
// SonorousLookAndFeel. The editor's monoPrintLookAndFeel member is now a
// SonorousLookAndFeel instance, so all draw routines come from the kit.

//==============================================================================
// MainContentComponent Implementation
//==============================================================================
void GrainsAudioProcessorEditor::MainContentComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(paper));

    if (!grainDesignBounds.isEmpty())
    {
        g.setColour(juce::Colour(ink));
        g.drawRect(grainDesignBounds.reduced(4, 4), 1);
    }

    if (!filtersBounds.isEmpty())
    {
        g.setColour(juce::Colour(ink));
        g.drawRect(filtersBounds.reduced(4, 4), 1);
    }

    if (!outputBounds.isEmpty())
    {
        g.setColour(juce::Colour(ink));
        g.drawRect(outputBounds.reduced(4, 4), 1);
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
    closeButton.setButtonText("CLOSE");
    closeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(paper));
    closeButton.setColour(juce::TextButton::textColourOffId, juce::Colour(ink));
    closeButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(ink));
    closeButton.setColour(juce::TextButton::textColourOnId, juce::Colour(paper));
    addAndMakeVisible(closeButton);

    // Setup tabbed component
    tabbedComponent.setTabBarDepth(30);
    tabbedComponent.setColour(juce::TabbedComponent::backgroundColourId, juce::Colour(paper));
    tabbedComponent.setColour(juce::TabbedComponent::outlineColourId, juce::Colour(ink));
    tabbedComponent.setColour(juce::TabbedButtonBar::tabOutlineColourId, juce::Colour(ink));
    tabbedComponent.setColour(juce::TabbedButtonBar::frontOutlineColourId, juce::Colour(ink));
    tabbedComponent.setColour(juce::TabbedButtonBar::tabTextColourId, juce::Colour(quietInk));
    tabbedComponent.setColour(juce::TabbedButtonBar::frontTextColourId, juce::Colour(ink));
    addAndMakeVisible(tabbedComponent);

    auto styleSlider = [](juce::Slider& slider)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setRotaryParameters(juce::MathConstants<float>::pi * 0.75f,
                                   juce::MathConstants<float>::pi * 2.25f,
                                   true);
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(ink));
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    };

    auto styleLabel = [](juce::Label& label)
    {
        label.setFont(juce::Font(juce::FontOptions(8.0f)));
        label.setColour(juce::Label::textColourId, juce::Colour(quietInk));
        label.setJustificationType(juce::Justification::centred);
    };

    auto styleHeader = [](juce::Label& label)
    {
        label.setFont(juce::Font(juce::FontOptions(8.5f)));
        label.setColour(juce::Label::textColourId, juce::Colour(ink));
        label.setJustificationType(juce::Justification::left);
    };

    auto styleToggle = [](juce::ToggleButton& toggle)
    {
        toggle.setColour(juce::ToggleButton::textColourId, juce::Colour(ink));
        toggle.setColour(juce::ToggleButton::tickColourId, juce::Colour(ink));
        toggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(quietInk));
    };

    auto styleCombo = [](juce::ComboBox& combo)
    {
        combo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(paper));
        combo.setColour(juce::ComboBox::textColourId, juce::Colour(ink));
        combo.setColour(juce::ComboBox::outlineColourId, juce::Colour(ink));
        combo.setColour(juce::ComboBox::arrowColourId, juce::Colour(ink));
    };

    // Create tab content components
    modulationTab = std::make_unique<ModulationTab>(*this);
    soundDesignTab = std::make_unique<SoundDesignTab>(*this);

    // Add tabs
    tabbedComponent.addTab("Patch", juce::Colour(paper), modulationTab.get(), false);
    tabbedComponent.addTab("Treatment", juce::Colour(paper), soundDesignTab.get(), false);

    // Setup LFO controls (styling only - added to content in ModulationTab constructor)
    for (int i = 0; i < 4; ++i)
    {
        auto& controls = lfoControls[i];
        juce::String lfoNum = juce::String(i + 1);

        styleSlider(controls.rateSlider);

        controls.rateLabel.setText("LFO " + lfoNum + " / Rate", juce::dontSendNotification);
        styleLabel(controls.rateLabel);

        styleSlider(controls.depthSlider);

        controls.depthLabel.setText("Depth", juce::dontSendNotification);
        styleLabel(controls.depthLabel);

        // Enabled toggle button
        controls.enabledToggle.setButtonText("ON");
        styleToggle(controls.enabledToggle);

        // Waveform combo
        controls.waveformCombo.addItem("Sine", 1);
        controls.waveformCombo.addItem("Triangle", 2);
        controls.waveformCombo.addItem("Saw Up", 3);
        controls.waveformCombo.addItem("Saw Down", 4);
        controls.waveformCombo.addItem("Square", 5);
        controls.waveformCombo.addItem("Random", 6);
        styleCombo(controls.waveformCombo);

        controls.waveformLabel.setText("Trace", juce::dontSendNotification);
        styleLabel(controls.waveformLabel);

        // Target combo
        controls.targetCombo.addItem("Focus", 1);
        controls.targetCombo.addItem("Scatter", 2);
        controls.targetCombo.addItem("Thread", 3);
        controls.targetCombo.addItem("Cloud", 4);
        controls.targetCombo.addItem("Lift", 5);
        controls.targetCombo.addItem("Drift", 6);
        controls.targetCombo.addItem("Halo", 7);
        controls.targetCombo.addItem("Reverse", 8);
        controls.targetCombo.addItem("None", 9);
        styleCombo(controls.targetCombo);

        controls.targetLabel.setText("Field", juce::dontSendNotification);
        styleLabel(controls.targetLabel);
    }

    // Setup Sound Design controls (styling only - added to content in SoundDesignTab constructor)
    styleSlider(timeStretchSlider);
    timeStretchSlider.setTooltip("Playback speed multiplier (0.25x-4x). Independent from pitch when unlocked. 1x = normal speed.");

    timeStretchLabel.setText("Pull", juce::dontSendNotification);
    styleLabel(timeStretchLabel);

    // Time Stretch toggle (pitch/time lock)
    timeStretchToggle.setButtonText("LOCK");
    styleToggle(timeStretchToggle);
    timeStretchToggle.setTooltip("When enabled: pitch and time are locked together (classic behavior). Disabled = independent control.");

    timeStretchToggleLabel.setText("Couple", juce::dontSendNotification);
    styleLabel(timeStretchToggleLabel);

    // Reverse Probability slider
    styleSlider(reverseProbabilitySlider);
    reverseProbabilitySlider.setTooltip("Probability that grains play backwards (0-100%). 0% = all forward, 100% = all reverse, 50% = random mix.");

    reverseProbabilityLabel.setText("Return", juce::dontSendNotification);
    styleLabel(reverseProbabilityLabel);

    // Octave Spread slider
    styleSlider(octaveSpreadSlider);
    octaveSpreadSlider.setTooltip("Octave shift range (0-2 octaves). Grains randomly shift ±spread. Creates layered harmonic textures.");

    octaveSpreadLabel.setText("Octaves", juce::dontSendNotification);
    styleLabel(octaveSpreadLabel);

    // Octave Probability slider
    styleSlider(octaveProbabilitySlider);
    octaveProbabilitySlider.setTooltip("Chance of octave shifting per grain (0-100%). Higher = more grains affected by octave spread.");

    octaveProbabilityLabel.setText("Chance", juce::dontSendNotification);
    styleLabel(octaveProbabilityLabel);

    // 3rd Octave Probability slider
    styleSlider(thirdOctaveProbSlider);
    thirdOctaveProbSlider.setTooltip("Probability of 3rd octave shift when Octave Spread >= 3. Allows extended harmonic range.");

    thirdOctaveProbLabel.setText("High", juce::dontSendNotification);
    styleLabel(thirdOctaveProbLabel);

    // =========================================================================
    // Sound Quality / Randomization controls (ported from grains-vst)
    // =========================================================================

    // Filter Randomization slider
    styleSlider(filterRandomizationSlider);
    filterRandomizationSlider.setTooltip("Per-grain filter cutoff randomization (0-100%). Creates timbral variation and warmth.");

    filterRandomizationLabel.setText("Vein", juce::dontSendNotification);
    styleLabel(filterRandomizationLabel);

    // Detune slider
    styleSlider(detuneSlider);
    detuneSlider.setTooltip("Per-grain micro-detuning in cents (0-50). Adds warmth and chorus-like richness.");

    detuneLabel.setText("Blur", juce::dontSendNotification);
    styleLabel(detuneLabel);

    // Jitter slider
    styleSlider(jitterSlider);
    jitterSlider.setTooltip("Timing humanization (0-100%). Higher = more organic/loose timing, lower = tighter sync.");

    jitterLabel.setText("Tremor", juce::dontSendNotification);
    styleLabel(jitterLabel);

    // Size Randomization slider
    styleSlider(sizeRandomizationSlider);
    sizeRandomizationSlider.setTooltip("Per-grain size variation (0-100%). Creates more varied, organic textures.");

    sizeRandomizationLabel.setText("Fray", juce::dontSendNotification);
    styleLabel(sizeRandomizationLabel);

    // Section header labels for Sound Design tab
    playbackSectionLabel.setText("TRANSPORT", juce::dontSendNotification);
    styleHeader(playbackSectionLabel);

    octavesSectionLabel.setText("HARMONIC SHEAR", juce::dontSendNotification);
    styleHeader(octavesSectionLabel);

    soundQualitySectionLabel.setText("SURFACE NOISE", juce::dontSendNotification);
    styleHeader(soundQualitySectionLabel);
}

void GrainsAudioProcessorEditor::AdvancedPanel::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(paper).withAlpha(0.98f));

    for (int x = 12; x < getWidth(); x += 28)
        for (int y = 14; y < getHeight(); y += 28)
        {
            g.setColour(juce::Colour(faintInk));
            g.fillEllipse(static_cast<float>(x), static_cast<float>(y), 0.9f, 0.9f);
        }

    auto header = getLocalBounds().removeFromTop(50).reduced(10, 0);
    g.setColour(juce::Colour(ink));
    g.drawRect(getLocalBounds(), 2);
    g.drawLine(0.0f, 49.5f, static_cast<float>(getWidth()), 49.5f, 1.2f);

    auto title = header;
    title.removeFromLeft(84);
    g.setFont(juce::Font(juce::FontOptions(9.0f)));
    g.drawText("REAR PLATE / SPECIMEN CALIBRATION", title, juce::Justification::centredLeft);
}

void GrainsAudioProcessorEditor::AdvancedPanel::resized()
{
    // Close button in top left
    closeButton.setBounds(10, 11, 68, 28);

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
    contentComponent.addAndMakeVisible(panel.thirdOctaveProbSlider);
    contentComponent.addAndMakeVisible(panel.thirdOctaveProbLabel);
    contentComponent.addAndMakeVisible(panel.grainShapeCombo);
    contentComponent.addAndMakeVisible(panel.grainShapeLabel);

    // Sound quality / randomization controls (ported from grains-vst)
    contentComponent.addAndMakeVisible(panel.filterRandomizationSlider);
    contentComponent.addAndMakeVisible(panel.filterRandomizationLabel);
    contentComponent.addAndMakeVisible(panel.detuneSlider);
    contentComponent.addAndMakeVisible(panel.detuneLabel);
    contentComponent.addAndMakeVisible(panel.jitterSlider);
    contentComponent.addAndMakeVisible(panel.jitterLabel);
    contentComponent.addAndMakeVisible(panel.sizeRandomizationSlider);
    contentComponent.addAndMakeVisible(panel.sizeRandomizationLabel);

    // Section headers
    contentComponent.addAndMakeVisible(panel.playbackSectionLabel);
    contentComponent.addAndMakeVisible(panel.octavesSectionLabel);
    contentComponent.addAndMakeVisible(panel.soundQualitySectionLabel);
}

void GrainsAudioProcessorEditor::AdvancedPanel::SoundDesignTab::resized()
{
    // Prevent recursive resize calls
    if (isResizing)
        return;

    juce::ScopedValueSetter<bool> resizeGuard(isResizing, true);

    // Fill the tab with the viewport
    viewport.setBounds(getLocalBounds());

    const int margin = 20;
    const int knobSize = 60;  // Smaller knobs for compact layout
    const int labelHeight = 14;
    const int sectionHeaderHeight = 18;
    const int sectionGap = 25;
    const int knobRowHeight = knobSize + labelHeight + 8;
    const int contentWidth = getWidth();

    int yPos = margin;

    // Calculate 3-column positions (for knobs)
    int usableWidth = contentWidth - (2 * margin);
    int colSpacing = usableWidth / 3;
    int col1X = margin + colSpacing / 2 - knobSize / 2;
    int col2X = margin + colSpacing + colSpacing / 2 - knobSize / 2;
    int col3X = margin + 2 * colSpacing + colSpacing / 2 - knobSize / 2;

    // Calculate 4-column positions (for sound quality section)
    int col4Spacing = usableWidth / 4;
    int col4_1X = margin + col4Spacing / 2 - knobSize / 2;
    int col4_2X = margin + col4Spacing + col4Spacing / 2 - knobSize / 2;
    int col4_3X = margin + 2 * col4Spacing + col4Spacing / 2 - knobSize / 2;
    int col4_4X = margin + 3 * col4Spacing + col4Spacing / 2 - knobSize / 2;

    // =========================================================================
    // PLAYBACK SECTION
    // =========================================================================
    panel.playbackSectionLabel.setBounds(margin, yPos, usableWidth, sectionHeaderHeight);
    yPos += sectionHeaderHeight + 8;

    // Row: Time Stretch | Reverse Prob | Grain Shape
    panel.timeStretchLabel.setBounds(col1X - 5, yPos, knobSize + 10, labelHeight);
    panel.timeStretchSlider.setBounds(col1X, yPos + labelHeight + 2, knobSize, knobSize);

    panel.reverseProbabilityLabel.setBounds(col2X - 15, yPos, knobSize + 30, labelHeight);
    panel.reverseProbabilitySlider.setBounds(col2X, yPos + labelHeight + 2, knobSize, knobSize);

    panel.grainShapeLabel.setBounds(col3X - 5, yPos, knobSize + 10, labelHeight);
    panel.grainShapeCombo.setBounds(col3X - 20, yPos + labelHeight + 2 + 15, knobSize + 40, 28);

    yPos += knobRowHeight;

    // Pitch/Time Lock toggle (below Time Stretch)
    panel.timeStretchToggleLabel.setBounds(col1X - 15, yPos, knobSize + 30, labelHeight);
    panel.timeStretchToggle.setBounds(col1X - 5, yPos + labelHeight + 2, 70, 24);

    yPos += labelHeight + 30 + sectionGap;

    // Store divider position (between Playback and Octaves)
    divider1Y = yPos - sectionGap / 2;

    // =========================================================================
    // OCTAVES SECTION
    // =========================================================================
    panel.octavesSectionLabel.setBounds(margin, yPos, usableWidth, sectionHeaderHeight);
    yPos += sectionHeaderHeight + 8;

    // Row: Octave Spread | Octave Prob | 3rd Oct Prob
    panel.octaveSpreadLabel.setBounds(col1X - 15, yPos, knobSize + 30, labelHeight);
    panel.octaveSpreadSlider.setBounds(col1X, yPos + labelHeight + 2, knobSize, knobSize);

    panel.octaveProbabilityLabel.setBounds(col2X - 15, yPos, knobSize + 30, labelHeight);
    panel.octaveProbabilitySlider.setBounds(col2X, yPos + labelHeight + 2, knobSize, knobSize);

    panel.thirdOctaveProbLabel.setBounds(col3X - 15, yPos, knobSize + 30, labelHeight);
    panel.thirdOctaveProbSlider.setBounds(col3X, yPos + labelHeight + 2, knobSize, knobSize);

    yPos += knobRowHeight + sectionGap;

    // Store divider position (between Octaves and Sound Quality)
    divider2Y = yPos - sectionGap / 2;

    // =========================================================================
    // SOUND QUALITY SECTION (4-column layout)
    // =========================================================================
    panel.soundQualitySectionLabel.setBounds(margin, yPos, usableWidth, sectionHeaderHeight);
    yPos += sectionHeaderHeight + 8;

    // Row: Filter Random | Detune | Jitter | Size Random
    panel.filterRandomizationLabel.setBounds(col4_1X - 15, yPos, knobSize + 30, labelHeight);
    panel.filterRandomizationSlider.setBounds(col4_1X, yPos + labelHeight + 2, knobSize, knobSize);

    panel.detuneLabel.setBounds(col4_2X - 5, yPos, knobSize + 10, labelHeight);
    panel.detuneSlider.setBounds(col4_2X, yPos + labelHeight + 2, knobSize, knobSize);

    panel.jitterLabel.setBounds(col4_3X - 5, yPos, knobSize + 10, labelHeight);
    panel.jitterSlider.setBounds(col4_3X, yPos + labelHeight + 2, knobSize, knobSize);

    panel.sizeRandomizationLabel.setBounds(col4_4X - 15, yPos, knobSize + 30, labelHeight);
    panel.sizeRandomizationSlider.setBounds(col4_4X, yPos + labelHeight + 2, knobSize, knobSize);

    yPos += knobRowHeight + margin;

    // Calculate total content height needed
    int totalHeight = yPos;
    contentComponent.setSize(contentWidth, totalHeight);

    // Repaint to draw dividers
    contentComponent.repaint();
}

//==============================================================================
// Main Editor Implementation
//==============================================================================
GrainsAudioProcessorEditor::GrainsAudioProcessorEditor (GrainsAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), tooltipWindow(this, 1500)
{
    // Fonts come from sonorous-kit; the local Ptr members keep their names so
    // the rest of the editor's monoFont(...)/serifFont(...) calls don't change.
    instrumentSerifRegular = sonorous::typography::instrumentSerifRegular();
    instrumentSerifItalic  = sonorous::typography::instrumentSerifItalic();
    jetBrainsMono          = sonorous::typography::jetBrainsMono();

    // Set up load sample button
    loadSampleButton.setButtonText("+ Load Sample");
    loadSampleButton.addListener(this);
    loadSampleButton.setLookAndFeel(&monoPrintLookAndFeel);
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
    grainSourceSectionLabel.setText("GRAIN ENGINE", juce::dontSendNotification);
    grainSourceSectionLabel.setFont(monoFont(jetBrainsMono, 8.0f));
    grainSourceSectionLabel.setColour(juce::Label::textColourId, juce::Colour(quietInk));
    grainSourceSectionLabel.setJustificationType(juce::Justification::left);
    mainContentComponent.addAndMakeVisible(grainSourceSectionLabel);

    outputSectionLabel.setText("OUTPUT", juce::dontSendNotification);
    outputSectionLabel.setFont(monoFont(jetBrainsMono, 8.0f));
    outputSectionLabel.setColour(juce::Label::textColourId, juce::Colour(quietInk));
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
    setupLabel(positionLabel, "Focus");
    positionSlider.setTooltip("Playback position in the sample (0-100%). Shows as a window overlay on the waveform.");
    positionAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "position", positionSlider));

    setupSlider(spraySlider, "spray");
    setupLabel(sprayLabel, "Scatter");
    spraySlider.setTooltip("Randomizes grain position around the main Position value. Higher values = more variation.");
    sprayAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "spray", spraySlider));

    setupSlider(grainSizeSlider, "grainSize");
    setupLabel(grainSizeLabel, "Thread");
    grainSizeSlider.setTooltip("Duration of each grain in milliseconds (5-500ms). Shorter = more granular, longer = smoother.");
    grainSizeAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "grainSize", grainSizeSlider));

    setupSlider(densitySlider, "density");
    setupLabel(densityLabel, "Cloud");
    densitySlider.setTooltip("Number of grains per second (1-100). Higher values create denser, more continuous textures.");
    densityAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "density", densitySlider));

    // Output section sliders (pink color #f472b6)
    setupSlider(pitchSlider, "pitch");
    setupLabel(pitchLabel, "Lift");
    pitchSlider.setTooltip("Pitch shift in semitones (-24 to +24). Follows MIDI note pitch automatically.");
    pitchSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xfff472b6));  // Pink
    pitchSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff9a8d4));  // Light pink accent
    pitchAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "pitch", pitchSlider));

    setupSlider(panSlider, "pan");
    setupLabel(panLabel, "Drift");
    panSlider.setTooltip("Stereo position of grains. -100 = left, 0 = center, +100 = right.");
    panSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xfff472b6));  // Pink
    panSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff9a8d4));  // Light pink accent
    panAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "pan", panSlider));

    setupSlider(gainSlider, "gain");
    setupLabel(gainLabel, "Exposure");
    gainSlider.setTooltip("Output volume in decibels (-60 to +12 dB). Use MIDI velocity for per-note dynamics.");
    gainSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xfff472b6));  // Pink
    gainSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff9a8d4));  // Light pink accent
    gainAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "gain", gainSlider));

    // Phase 2 parameters
    setupSlider(panSpreadSlider, "panSpread");
    setupLabel(panSpreadLabel, "Halo");
    panSpreadSlider.setTooltip("Randomizes pan position for each grain (0-100%). Creates wider stereo field.");
    panSpreadSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xfff472b6));  // Pink
    panSpreadSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff9a8d4));  // Light pink accent
    panSpreadAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "panSpread", panSpreadSlider));

    // Reverse toggle (simple on/off)
    reverseToggle.setButtonText("REV");
    reverseToggle.setColour(juce::ToggleButton::textColourId, juce::Colour(ink));
    reverseToggle.setColour(juce::ToggleButton::tickColourId, juce::Colour(ink));
    reverseToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(quietInk));
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
    filterSectionLabel.setFont(monoFont(jetBrainsMono, 8.0f));
    filterSectionLabel.setColour(juce::Label::textColourId, juce::Colour(quietInk));
    filterSectionLabel.setJustificationType(juce::Justification::left);
    mainContentComponent.addAndMakeVisible(filterSectionLabel);

    // Low-pass cutoff
    setupSlider(lpFilterCutoffSlider, "lpFilterCutoff");
    setupLabel(lpFilterCutoffLabel, "Veil");
    lpFilterCutoffSlider.setTooltip("Low-pass cutoff frequency (20Hz - 20kHz). Frequencies above this are attenuated.");
    lpFilterCutoffAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "lpFilterCutoff", lpFilterCutoffSlider));

    // Low-pass resonance
    setupSlider(lpFilterResonanceSlider, "lpFilterResonance");
    setupLabel(lpFilterResonanceLabel, "Ring");
    lpFilterResonanceSlider.setTooltip("Low-pass resonance (Q factor). Emphasizes frequencies near the cutoff. Higher = sharper peak.");
    lpFilterResonanceAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "lpFilterResonance", lpFilterResonanceSlider));

    // High-pass cutoff
    setupSlider(hpFilterCutoffSlider, "hpFilterCutoff");
    setupLabel(hpFilterCutoffLabel, "Floor");
    hpFilterCutoffSlider.setTooltip("High-pass cutoff frequency (20Hz - 20kHz). Frequencies below this are attenuated.");
    hpFilterCutoffAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "hpFilterCutoff", hpFilterCutoffSlider));

    // High-pass resonance
    setupSlider(hpFilterResonanceSlider, "hpFilterResonance");
    setupLabel(hpFilterResonanceLabel, "Edge");
    hpFilterResonanceSlider.setTooltip("High-pass resonance (Q factor). Emphasizes frequencies near the cutoff. Higher = sharper peak.");
    hpFilterResonanceAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "hpFilterResonance", hpFilterResonanceSlider));

    // Advanced panel toggle button (styled with more contrast)
    advancedToggleButton.setButtonText("Rear Plate");
    advancedToggleButton.setTooltip("Open advanced panel for LFO modulation and sound design parameters.");
    advancedToggleButton.addListener(this);
    advancedToggleButton.setLookAndFeel(&monoPrintLookAndFeel);
    advancedToggleButton.setClickingTogglesState(false);
    addAndMakeVisible(advancedToggleButton);

    // Create advanced panel overlay (initially hidden - default closed)
    advancedPanel = std::make_unique<AdvancedPanel>();
    advancedPanel->setLookAndFeel(&monoPrintLookAndFeel);
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

    thirdOctaveProbAttachment.reset(new SliderAttachment(
        audioProcessor.getAPVTS(), "thirdOctaveProb", advancedPanel->thirdOctaveProbSlider));

    // Sound quality / randomization attachments (ported from grains-vst)
    filterRandomizationAttachment.reset(new SliderAttachment(
        audioProcessor.getAPVTS(), "filterRandomization", advancedPanel->filterRandomizationSlider));

    detuneAttachment.reset(new SliderAttachment(
        audioProcessor.getAPVTS(), "detuneCents", advancedPanel->detuneSlider));

    jitterAttachment.reset(new SliderAttachment(
        audioProcessor.getAPVTS(), "jitterPercent", advancedPanel->jitterSlider));

    sizeRandomizationAttachment.reset(new SliderAttachment(
        audioProcessor.getAPVTS(), "grainSizeRandomization", advancedPanel->sizeRandomizationSlider));

    timeStretchAttachment.reset(new SliderAttachment(
        audioProcessor.getAPVTS(), "timeStretch", advancedPanel->timeStretchSlider));

    // Grain Shape dropdown (styling only - added to content in SoundDesignTab constructor)
    advancedPanel->grainShapeCombo.addItem("Hann", 1);
    advancedPanel->grainShapeCombo.addItem("Triangle", 2);
    advancedPanel->grainShapeCombo.addItem("Trapezoid", 3);
    advancedPanel->grainShapeCombo.addItem("Exponential", 4);
    advancedPanel->grainShapeCombo.addItem("Gaussian", 5);
    advancedPanel->grainShapeCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(paper));
    advancedPanel->grainShapeCombo.setColour(juce::ComboBox::textColourId, juce::Colour(ink));
    advancedPanel->grainShapeCombo.setColour(juce::ComboBox::outlineColourId, juce::Colour(ink));
    advancedPanel->grainShapeCombo.setColour(juce::ComboBox::arrowColourId, juce::Colour(ink));
    advancedPanel->grainShapeCombo.setTooltip("Envelope shape for each grain. Hann = smooth, Triangle = sharper, Exponential = natural decay.");

    advancedPanel->grainShapeLabel.setText("Cut", juce::dontSendNotification);
    advancedPanel->grainShapeLabel.setColour(juce::Label::textColourId, juce::Colour(quietInk));
    advancedPanel->grainShapeLabel.setJustificationType(juce::Justification::centred);
    advancedPanel->grainShapeLabel.setFont(monoFont(jetBrainsMono, 8.0f));

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
    loadSampleButton.setLookAndFeel(nullptr);
    advancedToggleButton.removeListener(this);
    advancedToggleButton.setLookAndFeel(nullptr);
    advancedPanel->setLookAndFeel(nullptr);
    advancedPanel->timeStretchToggle.removeListener(this);
    advancedPanel->closeButton.removeListener(this);

    for (auto* child : mainContentComponent.getChildren())
        if (auto* slider = dynamic_cast<juce::Slider*>(child))
            slider->setLookAndFeel(nullptr);

    for (auto* child : getChildren())
        if (auto* button = dynamic_cast<juce::TextButton*>(child))
            button->setLookAndFeel(nullptr);
}

//==============================================================================
void GrainsAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour(paper));

    for (int x = 0; x < getWidth(); x += 32)
        for (int y = 0; y < getHeight(); y += 32)
        {
            g.setColour(juce::Colour(faintInk));
            g.fillEllipse(static_cast<float>(x), static_cast<float>(y), 1.0f, 1.0f);
        }

    auto header = getLocalBounds().removeFromTop(60);
    g.setColour(juce::Colour(ink));
    g.drawLine(0.0f, static_cast<float>(header.getBottom()) - 0.5f,
               static_cast<float>(getWidth()), static_cast<float>(header.getBottom()) - 0.5f, 1.4f);

    auto titleArea = header.reduced(22, 0);
    g.setFont(monoFont(jetBrainsMono, 9.0f));
    g.drawText("PLATE I - GRANULAR MICROSCOPE", titleArea.removeFromLeft(230),
               juce::Justification::centredLeft);

    g.setFont(serifFont(instrumentSerifItalic, 33.0f));
    g.drawText("Lamina", titleArea.removeFromLeft(180), juce::Justification::centredLeft);

    g.setFont(monoFont(jetBrainsMono, 8.0f));
    g.drawText("SPECIMEN / AUDIO SLIDE", titleArea, juce::Justification::centredLeft);

    // Draw drag-over overlay when dragging a file
    if (isDraggingFile)
    {
        g.setColour(juce::Colour(0xffb4a5d8).withAlpha(0.3f));
        g.fillAll();
        g.setColour(juce::Colour(ink));
        g.drawRect(getLocalBounds().reduced(10), 3);
        g.setFont(serifFont(instrumentSerifItalic, 28.0f));
        g.drawText("Drop specimen here", getLocalBounds(), juce::Justification::centred);
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
    const int adsrPlateOverlap = 4;
    const int adsrBottomGap = 4;
    const int adsrVerticalNudge = 8;
    int adsrHeight = filterSectionEndY - rightYPos - adsrBottomGap;  // Match filter end position, minus gap
    interactiveADSR.setBounds(rightX - adsrPlateOverlap, rightYPos + adsrVerticalNudge,
                              columnWidth + adsrPlateOverlap * 2, adsrHeight);

    rightYPos += adsrHeight + adsrBottomGap;

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

    // Position advanced panel overlay (left side, full height)
    // Always position it so it's ready when toggled visible
    if (advancedPanel != nullptr)
    {
        // Panel width adapts to window size (max 550px, min 450px)
        int panelWidth = juce::jlimit(450, 550, getWidth() - 50);
        advancedPanel->setBounds(0, 0, panelWidth, getHeight());
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
    juce::ignoreUnused(paramID);
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setRotaryParameters(juce::MathConstants<float>::pi * 0.75f,
                               juce::MathConstants<float>::pi * 2.25f,
                               true);
    slider.setLookAndFeel(&monoPrintLookAndFeel);
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(ink));
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

    slider.addListener(this);
    mainContentComponent.addAndMakeVisible(slider);
}

void GrainsAudioProcessorEditor::setupLabel (juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, juce::Colour(quietInk));
    label.setJustificationType(juce::Justification::centred);
    label.setFont(monoFont(jetBrainsMono, 7.4f));
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
