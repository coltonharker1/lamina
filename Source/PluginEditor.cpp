#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GrainsAudioProcessorEditor::GrainsAudioProcessorEditor (GrainsAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set up load sample button
    loadSampleButton.setButtonText("Load Sample");
    loadSampleButton.addListener(this);
    loadSampleButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff22c55e));
    loadSampleButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    addAndMakeVisible(loadSampleButton);

    // Sample status label
    sampleLabel.setText("No sample loaded", juce::dontSendNotification);
    sampleLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
    sampleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(sampleLabel);

    // Set up all sliders and labels
    setupSlider(positionSlider, "position");
    setupLabel(positionLabel, "Position");
    positionAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "position", positionSlider));

    setupSlider(spraySlider, "spray");
    setupLabel(sprayLabel, "Spray");
    sprayAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "spray", spraySlider));

    setupSlider(grainSizeSlider, "grainSize");
    setupLabel(grainSizeLabel, "Grain Size");
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

    setupSlider(dryWetSlider, "dryWet");
    setupLabel(dryWetLabel, "Dry/Wet");
    dryWetAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "dryWet", dryWetSlider));

    setupSlider(gainSlider, "gain");
    setupLabel(gainLabel, "Gain");
    gainAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "gain", gainSlider));

    // Phase 2 parameters
    setupSlider(panSpreadSlider, "panSpread");
    setupLabel(panSpreadLabel, "Pan Spread");
    panSpreadAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "panSpread", panSpreadSlider));

    setupSlider(pitchSpreadSlider, "pitchSpread");
    setupLabel(pitchSpreadLabel, "Pitch Spread");
    pitchSpreadAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "pitchSpread", pitchSpreadSlider));

    // Freeze button
    freezeButton.setButtonText("Freeze");
    freezeButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    freezeButton.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xff22c55e));
    freezeButton.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xff2a2a2a));
    addAndMakeVisible(freezeButton);
    setupLabel(freezeLabel, "Freeze");
    freezeAttachment.reset(new ButtonAttachment(audioProcessor.getAPVTS(), "freeze", freezeButton));

    setupSlider(reverseSlider, "reverse");
    setupLabel(reverseLabel, "Reverse");
    reverseAttachment.reset(new SliderAttachment(audioProcessor.getAPVTS(), "reverse", reverseSlider));

    // Set window size - larger to fit new controls
    setSize (1000, 600);
    setResizable(true, true);
    setResizeLimits(800, 500, 1400, 900);
}

GrainsAudioProcessorEditor::~GrainsAudioProcessorEditor()
{
    loadSampleButton.removeListener(this);
}

//==============================================================================
void GrainsAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Dark background matching web version
    g.fillAll (juce::Colour (0xff0a0a0a));

    // Title bar
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillRect(0, 0, getWidth(), 60);

    // Plugin title
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Arial", 32.0f, juce::Font::bold));
    g.drawText("Grains", 20, 10, 200, 40, juce::Justification::left);

    // Version
    g.setFont(juce::Font("Arial", 12.0f, juce::Font::plain));
    g.setColour(juce::Colours::grey);
    g.drawText("v0.1 MVP", 20, 45, 200, 15, juce::Justification::left);

    // Section dividers
    g.setColour(juce::Colour(0xff2a2a2a));
    g.drawLine(20, 140, getWidth() - 20, 140, 1.0f);
    g.drawLine(20, 340, getWidth() - 20, 340, 1.0f);
}

void GrainsAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    // Title area
    area.removeFromTop(60);

    // Load sample section
    auto loadArea = area.removeFromTop(80);
    loadSampleButton.setBounds(loadArea.reduced(20, 15).removeFromTop(35));
    sampleLabel.setBounds(loadArea.reduced(20, 5));

    // Sliders section - 2 rows of 4
    area.removeFromTop(10);  // Spacing

    // Row 1: Position, Spray, Grain Size, Density
    auto row1 = area.removeFromTop(180);
    int sliderWidth = (getWidth() - 60) / 4;

    positionLabel.setBounds(20, row1.getY(), sliderWidth, 20);
    positionSlider.setBounds(20, row1.getY() + 25, sliderWidth, 140);

    sprayLabel.setBounds(20 + sliderWidth, row1.getY(), sliderWidth, 20);
    spraySlider.setBounds(20 + sliderWidth, row1.getY() + 25, sliderWidth, 140);

    grainSizeLabel.setBounds(20 + sliderWidth * 2, row1.getY(), sliderWidth, 20);
    grainSizeSlider.setBounds(20 + sliderWidth * 2, row1.getY() + 25, sliderWidth, 140);

    densityLabel.setBounds(20 + sliderWidth * 3, row1.getY(), sliderWidth, 20);
    densitySlider.setBounds(20 + sliderWidth * 3, row1.getY() + 25, sliderWidth, 140);

    // Row 2: Pitch, Pan, Dry/Wet, Gain
    area.removeFromTop(10);  // Spacing
    auto row2 = area.removeFromTop(180);

    pitchLabel.setBounds(20, row2.getY(), sliderWidth, 20);
    pitchSlider.setBounds(20, row2.getY() + 25, sliderWidth, 140);

    panLabel.setBounds(20 + sliderWidth, row2.getY(), sliderWidth, 20);
    panSlider.setBounds(20 + sliderWidth, row2.getY() + 25, sliderWidth, 140);

    dryWetLabel.setBounds(20 + sliderWidth * 2, row2.getY(), sliderWidth, 20);
    dryWetSlider.setBounds(20 + sliderWidth * 2, row2.getY() + 25, sliderWidth, 140);

    gainLabel.setBounds(20 + sliderWidth * 3, row2.getY(), sliderWidth, 20);
    gainSlider.setBounds(20 + sliderWidth * 3, row2.getY() + 25, sliderWidth, 140);

    // Row 3: Phase 2 parameters (Pan Spread, Pitch Spread, Freeze, Reverse)
    area.removeFromTop(10);  // Spacing
    auto row3 = area.removeFromTop(180);

    panSpreadLabel.setBounds(20, row3.getY(), sliderWidth, 20);
    panSpreadSlider.setBounds(20, row3.getY() + 25, sliderWidth, 140);

    pitchSpreadLabel.setBounds(20 + sliderWidth, row3.getY(), sliderWidth, 20);
    pitchSpreadSlider.setBounds(20 + sliderWidth, row3.getY() + 25, sliderWidth, 140);

    // Freeze toggle button (smaller, centered vertically)
    freezeLabel.setBounds(20 + sliderWidth * 2, row3.getY(), sliderWidth, 20);
    freezeButton.setBounds(20 + sliderWidth * 2 + sliderWidth/2 - 30, row3.getY() + 60, 60, 30);

    reverseLabel.setBounds(20 + sliderWidth * 3, row3.getY(), sliderWidth, 20);
    reverseSlider.setBounds(20 + sliderWidth * 3, row3.getY() + 25, sliderWidth, 140);
}

//==============================================================================
void GrainsAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
    // Slider values are automatically synchronized via attachments
    // This callback is here in case we need to do anything extra
}

void GrainsAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    if (button == &loadSampleButton)
    {
        // Open file chooser (modern async API)
        auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

        auto fileChooser = std::make_shared<juce::FileChooser>(
            "Select an audio file to load...",
            juce::File::getSpecialLocation(juce::File::userHomeDirectory),
            "*.wav;*.aif;*.aiff;*.mp3");

        fileChooser->launchAsync(chooserFlags, [this, fileChooser](const juce::FileChooser& chooser)
        {
            juce::File file = chooser.getResult();

            if (file.existsAsFile())
            {
                audioProcessor.loadSampleFromFile(file);

                if (audioProcessor.isSampleLoaded())
                {
                    sampleLabel.setText(file.getFileNameWithoutExtension(), juce::dontSendNotification);
                    sampleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff22c55e));
                }
                else
                {
                    sampleLabel.setText("Failed to load sample", juce::dontSendNotification);
                    sampleLabel.setColour(juce::Label::textColourId, juce::Colours::red);
                }
            }
        });
    }
}

//==============================================================================
void GrainsAudioProcessorEditor::setupSlider (juce::Slider& slider, const juce::String& paramID)
{
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff22c55e));
    slider.setColour(juce::Slider::trackColourId, juce::Colour(0xff2a2a2a));
    slider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff1a1a1a));
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff1a1a1a));
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff2a2a2a));
    slider.addListener(this);
    addAndMakeVisible(slider);
}

void GrainsAudioProcessorEditor::setupLabel (juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, juce::Colours::white);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font("Arial", 14.0f, juce::Font::bold));
    addAndMakeVisible(label);
}
