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

    // Filter parameters - Low Pass
    const juce::String lpFilterEnabled { "lpFilterEnabled" };
    const juce::String lpFilterCutoff { "lpFilterCutoff" };
    const juce::String lpFilterResonance { "lpFilterResonance" };

    // Filter parameters - High Pass
    const juce::String hpFilterEnabled { "hpFilterEnabled" };
    const juce::String hpFilterCutoff { "hpFilterCutoff" };
    const juce::String hpFilterResonance { "hpFilterResonance" };

    // Octave randomization parameters
    const juce::String octaveSpread { "octaveSpread" };
    const juce::String octaveProbability { "octaveProbability" };
    const juce::String thirdOctaveProb { "thirdOctaveProb" };

    // Sound quality / randomization parameters (ported from grains-vst)
    const juce::String filterRandomization { "filterRandomization" };
    const juce::String detuneCents { "detuneCents" };
    const juce::String jitterPercent { "jitterPercent" };
    const juce::String grainSizeRandomization { "grainSizeRandomization" };
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
    cacheParameterPointers();
}

void GrainsAudioProcessor::cacheParameterPointers()
{
    auto get = [this](const juce::String& id) { return apvts.getRawParameterValue(id); };

    params.position           = get(ParameterIDs::position);
    params.spray              = get(ParameterIDs::spray);
    params.grainSize          = get(ParameterIDs::grainSize);
    params.density            = get(ParameterIDs::density);
    params.pitch              = get(ParameterIDs::pitch);
    params.pan                = get(ParameterIDs::pan);
    params.gain               = get(ParameterIDs::gain);

    params.panSpread          = get(ParameterIDs::panSpread);
    params.freeze             = get(ParameterIDs::freeze);
    params.reverseProbability = get(ParameterIDs::reverseProbability);

    params.timeStretch        = get(ParameterIDs::timeStretch);
    params.grainShape         = get(ParameterIDs::grainShape);
    params.pitchTimeLock      = get(ParameterIDs::pitchTimeLock);

    params.envAttack          = get(ParameterIDs::envAttack);
    params.envDecay           = get(ParameterIDs::envDecay);
    params.envSustain         = get(ParameterIDs::envSustain);
    params.envRelease         = get(ParameterIDs::envRelease);

    params.lpFilterEnabled    = get(ParameterIDs::lpFilterEnabled);
    params.lpFilterCutoff     = get(ParameterIDs::lpFilterCutoff);
    params.lpFilterResonance  = get(ParameterIDs::lpFilterResonance);

    params.hpFilterEnabled    = get(ParameterIDs::hpFilterEnabled);
    params.hpFilterCutoff     = get(ParameterIDs::hpFilterCutoff);
    params.hpFilterResonance  = get(ParameterIDs::hpFilterResonance);

    params.octaveSpread       = get(ParameterIDs::octaveSpread);
    params.octaveProbability  = get(ParameterIDs::octaveProbability);
    params.thirdOctaveProb    = get(ParameterIDs::thirdOctaveProb);

    params.filterRandomization    = get(ParameterIDs::filterRandomization);
    params.detuneCents            = get(ParameterIDs::detuneCents);
    params.jitterPercent          = get(ParameterIDs::jitterPercent);
    params.grainSizeRandomization = get(ParameterIDs::grainSizeRandomization);

    for (int i = 0; i < 4; ++i)
    {
        params.lfoRate[i]      = get(ParameterIDs::lfoRate(i));
        params.lfoDepth[i]     = get(ParameterIDs::lfoDepth(i));
        params.lfoWaveform[i]  = get(ParameterIDs::lfoWaveform(i));
        params.lfoTarget[i]    = get(ParameterIDs::lfoTarget(i));
        params.lfoTempoSync[i] = get(ParameterIDs::lfoTempoSync(i));
        params.lfoPhase[i]     = get(ParameterIDs::lfoPhase(i));
        params.lfoEnabled[i]   = get(ParameterIDs::lfoEnabled(i));
    }
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
        "Focus",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        50.0f,  // Default: middle of sample
        "%"
    ));

    // Spray: Random deviation from position (0-100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::spray,
        "Scatter",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        10.0f,  // Default: 10% randomization
        "%"
    ));

    // Grain Size: Duration of each grain (5-500ms)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::grainSize,
        "Thread",
        juce::NormalisableRange<float>(5.0f, 500.0f, 1.0f, 0.3f),  // Skewed toward smaller values
        100.0f,  // Default: 100ms
        "ms"
    ));

    // Density: Grains per second (1-100)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::density,
        "Cloud",
        juce::NormalisableRange<float>(1.0f, 100.0f, 0.1f, 0.5f),  // Skewed toward lower values
        20.0f,  // Default: 20 grains/sec
        " gr/s"
    ));

    // Pitch: Playback speed (-24 to +24 semitones)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::pitch,
        "Lift",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f),
        0.0f,  // Default: no pitch shift
        " st"
    ));

    // Pan: Stereo position (-100 to +100, 0 = center)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::pan,
        "Drift",
        juce::NormalisableRange<float>(-100.0f, 100.0f, 1.0f),
        0.0f,  // Default: center
        ""
    ));

    // Gain: Output level (0-200%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::gain,
        "Exposure",
        juce::NormalisableRange<float>(0.0f, 200.0f, 0.1f),
        100.0f,  // Default: 100% (unity gain)
        "%"
    ));

    // Pan Spread: Random deviation from pan position (0-100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::panSpread,
        "Halo",
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
        0.0f,  // Default: 0% (no reverse grains, user can increase to add)
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

    // Low-pass filter enabled (always on, transparent at max cutoff)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::lpFilterEnabled,
        "LP Filter Enabled",
        true  // Default: on (transparent at max cutoff)
    ));

    // Low-pass filter cutoff: 20Hz - 20kHz
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::lpFilterCutoff,
        "LP Filter Cutoff",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f),  // Skewed toward lower frequencies
        20000.0f,  // Default: 20kHz (maximum - transparent, passes all frequencies)
        "Hz"
    ));

    // Low-pass filter resonance: 0.1 - 10.0 (Q factor)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::lpFilterResonance,
        "LP Filter Resonance",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f, 0.5f),
        0.707f,  // Default: Butterworth (flat response)
        ""
    ));

    // High-pass filter enabled (always on, transparent at min cutoff)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        ParameterIDs::hpFilterEnabled,
        "HP Filter Enabled",
        true  // Default: on (transparent at min cutoff)
    ));

    // High-pass filter cutoff: 20Hz - 20kHz
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::hpFilterCutoff,
        "HP Filter Cutoff",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f),  // Skewed toward lower frequencies
        20.0f,  // Default: 20Hz (minimum - transparent, passes all frequencies)
        "Hz"
    ));

    // High-pass filter resonance: 0.1 - 10.0 (Q factor)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::hpFilterResonance,
        "HP Filter Resonance",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.01f, 0.5f),
        0.707f,  // Default: Butterworth (flat response)
        ""
    ));

    // Octave Spread: How many octaves to randomly shift (0-3 octaves)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::octaveSpread,
        "Octave Spread",
        juce::NormalisableRange<float>(0.0f, 3.0f, 0.1f),
        0.0f,  // Default: no octave shifting
        " oct"
    ));

    // Octave Probability: Chance of applying octave shift (0-100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::octaveProbability,
        "Octave Probability",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        0.0f,  // Default: 0% (disabled)
        "%"
    ));

    // 3rd Octave Probability: Chance of 3rd octave shift when octaveSpread >= 3 (0-100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::thirdOctaveProb,
        "3rd Octave Probability",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        0.0f,  // Default: 0% (disabled)
        "%"
    ));

    // =========================================================================
    // Sound Quality / Randomization Parameters (ported from grains-vst)
    // =========================================================================

    // Filter Randomization: Per-grain filter cutoff variation (0-100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::filterRandomization,
        "Filter Randomization",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        0.0f,  // Default: 0% (no per-grain filtering)
        "%"
    ));

    // Detune: Per-grain micro-detuning in cents (0-50)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::detuneCents,
        "Detune",
        juce::NormalisableRange<float>(0.0f, 50.0f, 0.1f),
        0.0f,  // Default: 0 cents (no detuning)
        " cents"
    ));

    // Jitter: Timing humanization / random offset (0-100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::jitterPercent,
        "Jitter",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        0.0f,  // Default: 0% (precise timing)
        "%"
    ));

    // Grain Size Randomization: Per-grain size variation (0-100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ParameterIDs::grainSizeRandomization,
        "Size Randomization",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        0.0f,  // Default: 0% (fixed size)
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

    // Initialize LFOs
    for (auto& lfo : lfos)
    {
        lfo.prepare(sampleRate);
    }

    // Initialize filter
    filter.prepare(sampleRate, samplesPerBlock);

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

    // Get parameter values via cached pointers (no per-block string lookups)
    float position    = params.position->load();
    float spray       = params.spray->load();
    float grainSize   = params.grainSize->load();
    float density     = params.density->load();
    float pitch       = params.pitch->load();
    float pan         = params.pan->load() / 100.0f;  // Convert to -1..1
    float gain        = params.gain->load() / 100.0f;  // Convert to multiplier

    // Phase 2 parameters
    float panSpread = params.panSpread->load();
    bool  freeze    = params.freeze->load() > 0.5f;

    // Reverse: use probability directly (0% = no reverse, 100% = all reverse)
    float reverse = params.reverseProbability->load();

    // Phase 3 parameters
    float timeStretch  = params.timeStretch->load();
    int   grainShape   = static_cast<int>(params.grainShape->load());
    bool  pitchTimeLock = params.pitchTimeLock->load() > 0.5f;

    // If pitch-time is locked, adjust time stretch to match pitch (legacy behavior)
    if (pitchTimeLock)
    {
        // When locked, timeStretch follows pitch ratio
        timeStretch = std::pow(2.0f, pitch / 12.0f);
    }

    // =========================================================================
    // Phase 5: Set envelope parameters
    // =========================================================================
    float envAttack  = params.envAttack->load();
    float envDecay   = params.envDecay->load();
    float envSustain = params.envSustain->load() / 100.0f;  // Convert to 0-1
    float envRelease = params.envRelease->load();

    grainEngine.setEnvelopeAttack(envAttack);
    grainEngine.setEnvelopeDecay(envDecay);
    grainEngine.setEnvelopeSustain(envSustain);
    grainEngine.setEnvelopeRelease(envRelease);

    // =========================================================================
    // Phase 4: Process LFOs and apply modulation
    // =========================================================================

    // Get current BPM from host (for tempo sync). Use the JUCE 7+ getPosition()
    // API — getCurrentPosition() is deprecated and silently returns false in
    // newer hosts (Logic, Cubase 13+), which would force tempo sync to default
    // to 120 BPM without warning.
    float currentBPM = 120.0f;
    if (auto* playHead = getPlayHead())
    {
        if (auto pos = playHead->getPosition())
        {
            if (auto bpm = pos->getBpm())
                currentBPM = static_cast<float>(*bpm);
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
        bool lfoEnabled = params.lfoEnabled[i]->load() > 0.5f;
        if (!lfoEnabled)
            continue;  // Skip this LFO if disabled

        // Get LFO parameters
        float lfoRate     = params.lfoRate[i]->load();
        float lfoDepth    = params.lfoDepth[i]->load() / 100.0f;  // Convert to 0-1
        int   lfoWaveform = static_cast<int>(params.lfoWaveform[i]->load());
        int   lfoTarget   = static_cast<int>(params.lfoTarget[i]->load());
        bool  lfoTempoSync = params.lfoTempoSync[i]->load() > 0.5f;
        float lfoPhase    = params.lfoPhase[i]->load();

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

    // Get octave parameters
    float octaveSpread      = params.octaveSpread->load();
    float octaveProbability = params.octaveProbability->load();
    float thirdOctaveProb   = params.thirdOctaveProb->load();

    // Get sound quality / randomization parameters
    float filterRandomization    = params.filterRandomization->load();
    float detuneCents            = params.detuneCents->load();
    float jitterPercent          = params.jitterPercent->load();
    float grainSizeRandomization = params.grainSizeRandomization->load();

    // Generate grains for any active notes
    grainEngine.process(buffer, position / 100.0f, spray, grainSize, density,
                       pitch, 0.0f, pan, panSpread, freeze, reverse, timeStretch, grainShape,
                       octaveSpread, octaveProbability, thirdOctaveProb,
                       filterRandomization, detuneCents, jitterPercent, grainSizeRandomization);

    // =========================================================================
    // Apply Filters (Low-pass and High-pass can be active simultaneously)
    // =========================================================================

    // Low-pass filter
    bool  lpEnabled   = params.lpFilterEnabled->load() > 0.5f;
    float lpCutoff    = params.lpFilterCutoff->load();
    float lpResonance = params.lpFilterResonance->load();

    filter.setLowPassEnabled(lpEnabled);
    filter.setLowPassCutoff(lpCutoff);
    filter.setLowPassResonance(lpResonance);

    // High-pass filter
    bool  hpEnabled   = params.hpFilterEnabled->load() > 0.5f;
    float hpCutoff    = params.hpFilterCutoff->load();
    float hpResonance = params.hpFilterResonance->load();

    filter.setHighPassEnabled(hpEnabled);
    filter.setHighPassCutoff(hpCutoff);
    filter.setHighPassResonance(hpResonance);

    filter.process(buffer);

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
    // Build the new buffer entirely on the message thread, with no contact
    // with the audio thread. Reallocations, file reads and the mono mixdown
    // all happen on a local AudioBuffer that the audio thread cannot see.
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (reader == nullptr)
    {
        DBG("Failed to load sample: " + file.getFileName());
        return;
    }

    juce::AudioBuffer<float> newBuffer(static_cast<int>(reader->numChannels),
                                       static_cast<int>(reader->lengthInSamples));
    reader->read(&newBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);

    if (newBuffer.getNumChannels() > 1)
    {
        newBuffer.addFrom(0, 0, newBuffer, 1, 0, newBuffer.getNumSamples(), 0.5f);
        newBuffer.applyGain(0, 0, newBuffer.getNumSamples(), 0.5f);
        newBuffer.setSize(1, newBuffer.getNumSamples(), true, false, false);
    }

    // Atomic handoff: take the audio callback lock so processBlock cannot run
    // while we move the new data into sampleBuffer and re-point the engine.
    // The engine's setSourceBuffer() also deactivates every active voice, so
    // no voice keeps reading the old underlying float* after this returns.
    {
        const juce::ScopedLock sl(getCallbackLock());
        sampleBuffer = std::move(newBuffer);
        grainEngine.setSourceBuffer(sampleBuffer);
        sampleLoaded.store(true);
    }

    DBG("Loaded sample: " + file.getFileName() + " (" +
        juce::String(sampleBuffer.getNumSamples()) + " samples)");
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GrainsAudioProcessor();
}
