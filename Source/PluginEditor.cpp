#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GrainsAudioProcessorEditor::GrainsAudioProcessorEditor (GrainsAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set the initial size of the plugin window
    setSize (800, 600);
}

GrainsAudioProcessorEditor::~GrainsAudioProcessorEditor()
{
}

//==============================================================================
void GrainsAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Fill the background with dark color (matching web version aesthetic)
    g.fillAll (juce::Colour (0xff0a0a0a));

    // Draw plugin title
    g.setColour (juce::Colours::white);
    g.setFont (32.0f);
    g.drawFittedText ("Grains VST", getLocalBounds(), juce::Justification::centred, 1);

    // Draw "Hello World" message
    g.setFont (16.0f);
    g.drawFittedText ("Plugin Loaded Successfully!",
                      getLocalBounds().reduced(0, 100),
                      juce::Justification::centred, 1);
}

void GrainsAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
