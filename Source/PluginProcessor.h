#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "GrainEngine.h"
#include "LFO.h"
#include "Filter.h"

//==============================================================================
/**
 * Grains VST - Granular Synthesis Plugin
 * Audio Processor - handles all audio processing and parameter management
 */
class GrainsAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    GrainsAudioProcessor();
    ~GrainsAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Public access to parameters for UI binding
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Sample loading (public for UI access)
    void loadSampleFromFile(const juce::File& file);
    bool isSampleLoaded() const { return sampleLoaded.load(); }
    const juce::AudioBuffer<float>& getSampleBuffer() const { return sampleBuffer; }

    // Grain engine access (for visualizers)
    GrainEngine& getGrainEngine() { return grainEngine; }

    // LFO access (for UI)
    LFO& getLFO(int index) { return lfos[index]; }
    const LFO& getLFO(int index) const { return lfos[index]; }

    // Get modulated parameter values (for visual feedback)
    struct ModulatedValues
    {
        float position = 50.0f;
        float spray = 0.0f;
        float grainSize = 100.0f;
        float density = 20.0f;
        float pitch = 0.0f;
        float pan = 0.0f;
        float panSpread = 0.0f;
        float reverse = 0.0f;
    };

    const ModulatedValues& getModulatedValues() const { return modulatedValues; }

private:
    //==============================================================================
    // Parameter management
    juce::AudioProcessorValueTreeState apvts;

    // Cached APVTS parameter pointers — populated once in the constructor so the
    // audio thread doesn't string-lookup ~30 parameters per block.
    struct ParamPtrs
    {
        std::atomic<float>* position = nullptr;
        std::atomic<float>* spray = nullptr;
        std::atomic<float>* grainSize = nullptr;
        std::atomic<float>* density = nullptr;
        std::atomic<float>* pitch = nullptr;
        std::atomic<float>* pan = nullptr;
        std::atomic<float>* gain = nullptr;

        std::atomic<float>* panSpread = nullptr;
        std::atomic<float>* freeze = nullptr;
        std::atomic<float>* reverseProbability = nullptr;

        std::atomic<float>* timeStretch = nullptr;
        std::atomic<float>* grainShape = nullptr;
        std::atomic<float>* pitchTimeLock = nullptr;

        std::atomic<float>* envAttack = nullptr;
        std::atomic<float>* envDecay = nullptr;
        std::atomic<float>* envSustain = nullptr;
        std::atomic<float>* envRelease = nullptr;

        std::atomic<float>* lpFilterEnabled = nullptr;
        std::atomic<float>* lpFilterCutoff = nullptr;
        std::atomic<float>* lpFilterResonance = nullptr;

        std::atomic<float>* hpFilterEnabled = nullptr;
        std::atomic<float>* hpFilterCutoff = nullptr;
        std::atomic<float>* hpFilterResonance = nullptr;

        std::atomic<float>* octaveSpread = nullptr;
        std::atomic<float>* octaveProbability = nullptr;
        std::atomic<float>* thirdOctaveProb = nullptr;

        std::atomic<float>* filterRandomization = nullptr;
        std::atomic<float>* detuneCents = nullptr;
        std::atomic<float>* jitterPercent = nullptr;
        std::atomic<float>* grainSizeRandomization = nullptr;

        // LFO 1..4
        std::array<std::atomic<float>*, 4> lfoRate     {};
        std::array<std::atomic<float>*, 4> lfoDepth    {};
        std::array<std::atomic<float>*, 4> lfoWaveform {};
        std::array<std::atomic<float>*, 4> lfoTarget   {};
        std::array<std::atomic<float>*, 4> lfoTempoSync{};
        std::array<std::atomic<float>*, 4> lfoPhase    {};
        std::array<std::atomic<float>*, 4> lfoEnabled  {};
    } params;

    void cacheParameterPointers();

    // Granular synthesis engine
    GrainEngine grainEngine;

    // Modulation system
    std::array<LFO, 4> lfos;  // 4 independent LFOs
    ModulatedValues modulatedValues;  // Store modulated values for UI feedback

    // Filter
    Filter filter;

    // Audio sample buffer
    juce::AudioBuffer<float> sampleBuffer;
    std::atomic<bool> sampleLoaded { false };

    // Processing state
    double currentSampleRate = 44100.0;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GrainsAudioProcessor)
};
