#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "ModalPlate.h"
#include "AllPassFilter.h"
#include "Params.h"
#include "CircularBeamMesh.h"
// #include "FEMembrane.h"
//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

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

//    CircularMesh head = CircularMesh(8, 8, 0.2, 12, 3, 6, 16);
   CircularBeamMesh head = CircularBeamMesh(1, 
                                            10, 
                                            8,   
                                            1,  //A 
                                            Params::E_DEFAULT,
                                            Params::I_DEFAULT,
                                            Params::G_DEFAULT,  //G
                                            Params::p_DEFAULT);//p
   int doom_count = 0;
   int tek_count = 0;

   AllPassFilter filter = AllPassFilter();

    // UI PARAMETERS 
    //==============================================================================
    Params::realtime_params rt_params;
    Params::offline_params ol_params;

    Eigen::ArrayXd force_pattern = Eigen::ArrayXd::Zero(0);
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
