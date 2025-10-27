#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

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

private:
    //==============================================================================
    // Parameter management
    juce::AudioProcessorValueTreeState apvts;

    // Audio sample buffer
    juce::AudioBuffer<float> sampleBuffer;
    std::atomic<bool> sampleLoaded { false };

    // Processing state
    double currentSampleRate = 44100.0;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GrainsAudioProcessor)
};
