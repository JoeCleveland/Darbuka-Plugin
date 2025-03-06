#pragma once

#include "PluginProcessor.h"
#include "RenderView.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                              private juce::Slider::Listener,
                                              private juce::Timer
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    void sliderValueChanged (juce::Slider* slider) override;

    RenderView renderView;
    AudioPluginAudioProcessor& processorRef;

    juce::Slider filter_cut;

    juce::Slider membrane_decay;

    juce::Slider membrane_youngs_mod;
    juce::Slider membrane_moment_inert;
    juce::Slider membrane_mass_density;
    juce::Slider membrane_spoke_length;
    juce::Slider membrane_crust_ratio;

    juce::Slider bending_force;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
