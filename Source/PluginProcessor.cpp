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
    const juce::String gain { "gain" };

    // Phase 2 parameters
    const juce::String panSpread { "panSpread" };
    const juce::String freeze { "freeze" };
    const juce::String reverseEnabled { "reverseEnabled" };      // Simple toggle for main UI
    const juce::String reverseProbability { "reverseProbability" };  // Advanced: 0-100% probability

    // Phase 3 parameters
    const juce::String timeStretch { "timeStretch" };
    const juce::String grainShape { "grainShape" };
    const juce::String pitchTimeLock { "pitchTimeLock" };

    // Phase 4 parameters - LFO system
    juce::String lfoRate(int index) { return "lfo" + juce::String(index + 1) + "Rate"; }
    juce::String lfoDepth(int index) { return "lfo" + juce::String(index + 1) + "Depth"; }
    juce::String lfoWaveform(int index) { return "lfo" + juce::String(index + 1) + "Waveform"; }
    juce::String lfoTarget(int index) { return "lfo" + juce::String(index + 1) + "Target"; }
    juce::String lfoTempoSync(int index) { return "lfo" + juce::String(index + 1) + "TempoSync"; }
    juce::String lfoPhase(int index) { return "lfo" + juce::String(index + 1) + "Phase"; }
    juce::String lfoEnabled(int index) { return "lfo" + juce::String(index + 1) + "Enabled"; }

    // Phase 5 parameters - Envelope system
    const juce::String envAttack { "envAttack" };
    const juce::String envDecay { "envDecay" };
    const juce::String envSustain { "envSustain" };
    const juce::String envRelease { "envRelease" };
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

    // Freeze: Lock position for infinite grain generation
    layout.add(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::freeze,
        "Freeze",
        false  // Default: off
    ));

    // Reverse Enabled: Simple toggle for reverse mode (main UI)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::reverseEnabled,
        "Reverse",
        false  // Default: off (forward playback)
    ));

    // Reverse Probability: Advanced control - probability of playing grains in reverse (0-100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::reverseProbability,
        "Reverse Probability",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        100.0f,  // Default: 100% (all grains reversed when enabled)
        "%"
    ));

    // Time Stretch: Independent speed control (0.25x - 4x)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::timeStretch,
        "Time Stretch",
        juce::NormalisableRange<float>(0.25f, 4.0f, 0.01f, 0.3f),  // Skewed toward 1.0
        1.0f,  // Default: normal speed
        "x"
    ));

    // Grain Shape: Envelope window function (0-4: Hann, Triangle, Trapezoid, Exponential, Gaussian)
    layout.add(std::make_unique<juce::AudioParameterInt>(
        ParameterIDs::grainShape,
        "Grain Shape",
        0, 4,
        0  // Default: Hann window
    ));

    // Pitch-Time Lock: When true, pitch and time are coupled (legacy behavior)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::pitchTimeLock,
        "Pitch-Time Lock",
        true  // Default: locked (backward compatible)
    ));

    // =========================================================================
    // Phase 4: LFO Modulation System (4 LFOs)
    // =========================================================================
    for (int i = 0; i < 4; ++i)
    {
        juce::String lfoNum = juce::String(i + 1);

        // LFO Rate: 0.01 Hz - 20 Hz
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            ParameterIDs::lfoRate(i),
            "LFO " + lfoNum + " Rate",
            juce::NormalisableRange<float>(0.01f, 20.0f, 0.01f, 0.3f),  // Skewed toward slower rates
            1.0f,  // Default: 1 Hz
            " Hz"
        ));

        // LFO Depth: 0-100%
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            ParameterIDs::lfoDepth(i),
            "LFO " + lfoNum + " Depth",
            juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
            0.0f,  // Default: 0% (off)
            "%"
        ));

        // LFO Waveform: 0-5 (Sine, Triangle, Saw Up, Saw Down, Square, Random)
        layout.add(std::make_unique<juce::AudioParameterInt>(
            ParameterIDs::lfoWaveform(i),
            "LFO " + lfoNum + " Waveform",
            0, 5,
            0  // Default: Sine
        ));

        // LFO Target: 0-8 (Position, Spray, Size, Density, Pitch, Pan, PanSpread, Reverse, None)
        layout.add(std::make_unique<juce::AudioParameterInt>(
            ParameterIDs::lfoTarget(i),
            "LFO " + lfoNum + " Target",
            0, 8,
            8  // Default: None (off)
        ));

        // LFO Tempo Sync: Off by default
        layout.add(std::make_unique<juce::AudioParameterBool>(
            ParameterIDs::lfoTempoSync(i),
            "LFO " + lfoNum + " Tempo Sync",
            false  // Default: free-running
        ));

        // LFO Phase: 0-360 degrees
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            ParameterIDs::lfoPhase(i),
            "LFO " + lfoNum + " Phase",
            juce::NormalisableRange<float>(0.0f, 360.0f, 1.0f),
            0.0f,  // Default: 0 degrees
            "\u00b0"  // Degree symbol
        ));

        // LFO Enabled: On/Off toggle
        layout.add(std::make_unique<juce::AudioParameterBool>(
            ParameterIDs::lfoEnabled(i),
            "LFO " + lfoNum + " Enabled",
            false  // Default: off
        ));
    }

    // =========================================================================
    // Phase 5: ADSR Envelope System
    // =========================================================================

    // Attack: Time to reach peak (1-5000ms)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::envAttack,
        "Envelope Attack",
        juce::NormalisableRange<float>(1.0f, 5000.0f, 1.0f, 0.3f),  // Skewed toward lower values
        10.0f,  // Default: 10ms
        "ms"
    ));

    // Decay: Time to fall to sustain level (1-5000ms)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::envDecay,
        "Envelope Decay",
        juce::NormalisableRange<float>(1.0f, 5000.0f, 1.0f, 0.3f),
        300.0f,  // Default: 300ms
        "ms"
    ));

    // Sustain: Hold level while note is held (0-100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::envSustain,
        "Envelope Sustain",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        80.0f,  // Default: 80%
        "%"
    ));

    // Release: Time to fade out after note off (1-5000ms)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::envRelease,
        "Envelope Release",
        juce::NormalisableRange<float>(1.0f, 5000.0f, 1.0f, 0.3f),
        500.0f,  // Default: 500ms
        "ms"
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

    // Initialize LFOs
    for (auto& lfo : lfos)
    {
        lfo.prepare(sampleRate);
    }

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
    bool freeze = apvts.getRawParameterValue(ParameterIDs::freeze)->load() > 0.5f;

    // Reverse: combine toggle + probability
    bool reverseEnabled = apvts.getRawParameterValue(ParameterIDs::reverseEnabled)->load() > 0.5f;
    float reverseProbability = apvts.getRawParameterValue(ParameterIDs::reverseProbability)->load();
    float reverse = reverseEnabled ? reverseProbability : 0.0f;  // If disabled, 0% probability

    // Phase 3 parameters
    float timeStretch = apvts.getRawParameterValue(ParameterIDs::timeStretch)->load();
    int grainShape = static_cast<int>(apvts.getRawParameterValue(ParameterIDs::grainShape)->load());
    bool pitchTimeLock = apvts.getRawParameterValue(ParameterIDs::pitchTimeLock)->load() > 0.5f;

    // If pitch-time is locked, adjust time stretch to match pitch (legacy behavior)
    if (pitchTimeLock)
    {
        // When locked, timeStretch follows pitch ratio
        timeStretch = std::pow(2.0f, pitch / 12.0f);
    }

    // =========================================================================
    // Phase 5: Set envelope parameters
    // =========================================================================
    float envAttack = apvts.getRawParameterValue(ParameterIDs::envAttack)->load();
    float envDecay = apvts.getRawParameterValue(ParameterIDs::envDecay)->load();
    float envSustain = apvts.getRawParameterValue(ParameterIDs::envSustain)->load() / 100.0f;  // Convert to 0-1
    float envRelease = apvts.getRawParameterValue(ParameterIDs::envRelease)->load();

    grainEngine.setEnvelopeAttack(envAttack);
    grainEngine.setEnvelopeDecay(envDecay);
    grainEngine.setEnvelopeSustain(envSustain);
    grainEngine.setEnvelopeRelease(envRelease);

    // =========================================================================
    // Phase 4: Process LFOs and apply modulation
    // =========================================================================

    // Get current BPM from host (for tempo sync)
    auto playHead = getPlayHead();
    float currentBPM = 120.0f;  // Default BPM
    if (playHead != nullptr)
    {
        juce::AudioPlayHead::CurrentPositionInfo posInfo;
        if (playHead->getCurrentPosition(posInfo))
        {
            if (posInfo.bpm > 0.0)
                currentBPM = static_cast<float>(posInfo.bpm);
        }
    }

    // Create modulation accumulators for each parameter
    float positionMod = 0.0f;
    float sprayMod = 0.0f;
    float grainSizeMod = 0.0f;
    float densityMod = 0.0f;
    float pitchMod = 0.0f;
    float panMod = 0.0f;
    float panSpreadMod = 0.0f;
    float reverseMod = 0.0f;

    // Process each LFO
    for (int i = 0; i < 4; ++i)
    {
        // Check if LFO is enabled
        bool lfoEnabled = apvts.getRawParameterValue(ParameterIDs::lfoEnabled(i))->load() > 0.5f;
        if (!lfoEnabled)
            continue;  // Skip this LFO if disabled

        // Get LFO parameters
        float lfoRate = apvts.getRawParameterValue(ParameterIDs::lfoRate(i))->load();
        float lfoDepth = apvts.getRawParameterValue(ParameterIDs::lfoDepth(i))->load() / 100.0f;  // Convert to 0-1
        int lfoWaveform = static_cast<int>(apvts.getRawParameterValue(ParameterIDs::lfoWaveform(i))->load());
        int lfoTarget = static_cast<int>(apvts.getRawParameterValue(ParameterIDs::lfoTarget(i))->load());
        bool lfoTempoSync = apvts.getRawParameterValue(ParameterIDs::lfoTempoSync(i))->load() > 0.5f;
        float lfoPhase = apvts.getRawParameterValue(ParameterIDs::lfoPhase(i))->load();

        // Update LFO state
        lfos[i].setRate(lfoRate);
        lfos[i].setDepth(lfoDepth);
        lfos[i].setWaveform(static_cast<LFO::Waveform>(lfoWaveform));
        lfos[i].setPhaseOffset(lfoPhase);
        lfos[i].setTempoSync(lfoTempoSync);
        lfos[i].setTempoRate(currentBPM, 1.0f);  // Quarter note for now (could add parameter later)

        // Process LFO for this block
        lfos[i].process(buffer.getNumSamples());

        // Get LFO value (-depth to +depth)
        float lfoValue = lfos[i].getValue();

        // Apply to target parameter
        // Target: 0=Position, 1=Spray, 2=Size, 3=Density, 4=Pitch, 5=Pan, 6=PanSpread, 7=Reverse, 8=None
        switch (lfoTarget)
        {
            case 0: positionMod += lfoValue * 50.0f; break;      // Position: ±50% (0-100% range)
            case 1: sprayMod += lfoValue * 50.0f; break;         // Spray: ±50% (0-100% range)
            case 2: grainSizeMod += lfoValue * 247.5f; break;    // Size: ±247.5ms (5-500ms range)
            case 3: densityMod += lfoValue * 49.5f; break;       // Density: ±49.5 gr/s (1-100 range)
            case 4: pitchMod += lfoValue * 24.0f; break;         // Pitch: ±24 semitones (-24 to +24 range)
            case 5: panMod += lfoValue * 1.0f; break;            // Pan: ±1.0 (-1 to +1 range, -100 to +100 UI)
            case 6: panSpreadMod += lfoValue * 50.0f; break;     // PanSpread: ±50% (0-100% range)
            case 7: reverseMod += lfoValue * 50.0f; break;       // Reverse: ±50% (0-100% range)
            default: break;  // None
        }
    }

    // Apply modulation to parameters (clamp to valid ranges)
    position = juce::jlimit(0.0f, 100.0f, position + positionMod);
    spray = juce::jlimit(0.0f, 100.0f, spray + sprayMod);
    grainSize = juce::jlimit(5.0f, 500.0f, grainSize + grainSizeMod);
    density = juce::jlimit(1.0f, 100.0f, density + densityMod);
    pitch = juce::jlimit(-24.0f, 24.0f, pitch + pitchMod);
    pan = juce::jlimit(-1.0f, 1.0f, pan + panMod);
    panSpread = juce::jlimit(0.0f, 100.0f, panSpread + panSpreadMod);
    reverse = juce::jlimit(0.0f, 100.0f, reverse + reverseMod);

    // Store modulated values for UI feedback
    modulatedValues.position = position;
    modulatedValues.spray = spray;
    modulatedValues.grainSize = grainSize;
    modulatedValues.density = density;
    modulatedValues.pitch = pitch;
    modulatedValues.pan = pan * 100.0f;  // Convert back to -100..100 for UI
    modulatedValues.panSpread = panSpread;
    modulatedValues.reverse = reverse;

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
                       pitch, 0.0f, pan, panSpread, freeze, reverse, timeStretch, grainShape);

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
