#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_audio_formats/juce_audio_formats.h>

//==============================================================================
// Parameter IDs
namespace ParameterIDs
{
    const juce::String position { "position" };
    const juce::String spray { "spray" };
    const juce::String grainSize { "grainSize" };
    const juce::String density { "density" };
    const juce::String pitch { "pitch" };
    const juce::String pan { "pan" };
    const juce::String dryWet { "dryWet" };
    const juce::String gain { "gain" };

    // Phase 2 parameters
    const juce::String panSpread { "panSpread" };
    const juce::String pitchSpread { "pitchSpread" };
    const juce::String freeze { "freeze" };
    const juce::String reverse { "reverse" };
}

//==============================================================================
GrainsAudioProcessor::GrainsAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       apvts (*this, nullptr, "Parameters", createParameterLayout())
#endif
{
}

GrainsAudioProcessor::~GrainsAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout GrainsAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Position: Where in the sample grains spawn (0-100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::position,
        "Position",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        50.0f,  // Default: middle of sample
        "%"
    ));

    // Spray: Random deviation from position (0-100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::spray,
        "Spray",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        10.0f,  // Default: 10% randomization
        "%"
    ));

    // Grain Size: Duration of each grain (5-500ms)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::grainSize,
        "Grain Size",
        juce::NormalisableRange<float>(5.0f, 500.0f, 1.0f, 0.3f),  // Skewed toward smaller values
        100.0f,  // Default: 100ms
        "ms"
    ));

    // Density: Grains per second (1-100)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::density,
        "Density",
        juce::NormalisableRange<float>(1.0f, 100.0f, 0.1f, 0.5f),  // Skewed toward lower values
        20.0f,  // Default: 20 grains/sec
        " gr/s"
    ));

    // Pitch: Playback speed (-24 to +24 semitones)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::pitch,
        "Pitch",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f),
        0.0f,  // Default: no pitch shift
        " st"
    ));

    // Pan: Stereo position (-100 to +100, 0 = center)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::pan,
        "Pan",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 1.0f),
        0.0f,  // Default: center
        ""
    ));

    // Dry/Wet: Mix between dry signal and grains (0-100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::dryWet,
        "Dry/Wet",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        100.0f,  // Default: 100% wet
        "%"
    ));

    // Gain: Output level (0-200%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::gain,
        "Gain",
        juce::NormalisableRange<float>(0.0f, 200.0f, 0.1f),
        100.0f,  // Default: 100% (unity gain)
        "%"
    ));

    // Pan Spread: Random deviation from pan position (0-100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::panSpread,
        "Pan Spread",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        0.0f,  // Default: no randomization
        "%"
    ));

    // Pitch Spread: Random deviation from pitch (0-24 semitones)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::pitchSpread,
        "Pitch Spread",
        juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f),
        0.0f,  // Default: no randomization
        " st"
    ));

    // Freeze: Lock position for infinite grain generation
    layout.add(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::freeze,
        "Freeze",
        false  // Default: off
    ));

    // Reverse: Probability of playing grains in reverse (0-100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::reverse,
        "Reverse",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        0.0f,  // Default: no reverse grains
        "%"
    ));

    return layout;
}

//==============================================================================
const juce::String GrainsAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GrainsAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GrainsAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool GrainsAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double GrainsAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GrainsAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GrainsAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GrainsAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String GrainsAudioProcessor::getProgramName (int index)
{
    return {};
}

void GrainsAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void GrainsAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    // Initialize grain engine
    grainEngine.prepare(sampleRate, samplesPerBlock);

    if (sampleLoaded.load())
    {
        grainEngine.setSourceBuffer(sampleBuffer);
    }
}

void GrainsAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GrainsAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void GrainsAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear all output channels
    buffer.clear();

    // If no sample is loaded, output silence
    if (!sampleLoaded.load())
        return;

    // Get parameter values
    float position = apvts.getRawParameterValue(ParameterIDs::position)->load();
    float spray = apvts.getRawParameterValue(ParameterIDs::spray)->load();
    float grainSize = apvts.getRawParameterValue(ParameterIDs::grainSize)->load();
    float density = apvts.getRawParameterValue(ParameterIDs::density)->load();
    float pitch = apvts.getRawParameterValue(ParameterIDs::pitch)->load();
    float pan = apvts.getRawParameterValue(ParameterIDs::pan)->load() / 100.0f;  // Convert to -1..1
    float gain = apvts.getRawParameterValue(ParameterIDs::gain)->load() / 100.0f;  // Convert to multiplier

    // Phase 2 parameters
    float panSpread = apvts.getRawParameterValue(ParameterIDs::panSpread)->load();
    float pitchSpread = apvts.getRawParameterValue(ParameterIDs::pitchSpread)->load();
    bool freeze = apvts.getRawParameterValue(ParameterIDs::freeze)->load() > 0.5f;
    float reverse = apvts.getRawParameterValue(ParameterIDs::reverse)->load();

    // Process MIDI messages to track notes
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();

        if (message.isNoteOn())
        {
            // Note on: enable grain generation
            grainEngine.noteOn(message.getNoteNumber(), message.getVelocity());
        }
        else if (message.isNoteOff())
        {
            // Note off: disable grain generation for that note
            grainEngine.noteOff(message.getNoteNumber());
        }
    }

    // Generate grains for any active notes
    grainEngine.process(buffer, position / 100.0f, spray, grainSize, density,
                       pitch, pitchSpread, pan, panSpread, freeze, reverse);

    // Apply gain
    buffer.applyGain(gain);
}

//==============================================================================
bool GrainsAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GrainsAudioProcessor::createEditor()
{
    return new GrainsAudioProcessorEditor (*this);
}

//==============================================================================
void GrainsAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save parameter state
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void GrainsAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore parameter state
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
void GrainsAudioProcessor::loadSampleFromFile(const juce::File& file)
{
    // Create audio format manager
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    // Try to read the file
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

    if (reader != nullptr)
    {
        // Resize sample buffer
        sampleBuffer.setSize(reader->numChannels, static_cast<int>(reader->lengthInSamples));

        // Read audio data
        reader->read(&sampleBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);

        // If stereo, convert to mono by averaging channels
        if (sampleBuffer.getNumChannels() > 1)
        {
            sampleBuffer.addFrom(0, 0, sampleBuffer, 1, 0, sampleBuffer.getNumSamples(), 0.5f);
            sampleBuffer.applyGain(0, 0, sampleBuffer.getNumSamples(), 0.5f);
            sampleBuffer.setSize(1, sampleBuffer.getNumSamples(), true, false, false);
        }

        // Update grain engine
        grainEngine.setSourceBuffer(sampleBuffer);
        sampleLoaded.store(true);

        DBG("Loaded sample: " + file.getFileName() + " (" +
            juce::String(sampleBuffer.getNumSamples()) + " samples)");
    }
    else
    {
        DBG("Failed to load sample: " + file.getFileName());
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GrainsAudioProcessor();
}
