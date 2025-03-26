#pragma once
#include <juce_dsp/juce_dsp.h>

class DrumFilter {
public:
    DrumFilter();
    void getBlock(juce::AudioBuffer<float> buffer, float ratio, float wet);

private:
    juce::dsp::Convolution darbuka_conv_1;
    juce::dsp::Convolution darbuka_conv_2;
    static const float darbuka_1_IR[];
    static const float darbuka_2_IR[];
};