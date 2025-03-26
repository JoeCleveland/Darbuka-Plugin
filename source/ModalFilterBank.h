#pragma once
#include <juce_dsp/juce_dsp.h>
#include <Eigen/Dense>

class ModalFilterBank {
    ModalFilterBank();

    void setModes(Eigen::ArrayXd modes);
    void getBlock(juce::AudioBuffer<float> buffer);
};