#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);

    addAndMakeVisible(renderView);

    membrane_pitch.setSliderStyle (juce::Slider::LinearBarVertical);
    membrane_pitch.setRange (0.1, 5.0, 0.1);
    membrane_pitch.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    membrane_pitch.setPopupDisplayEnabled (true, false, this);
    membrane_pitch.setTextValueSuffix (" Membrane Pitch");
    membrane_pitch.setValue(1.0);
    addAndMakeVisible (&membrane_pitch);
    membrane_pitch.addListener(this);

    membrane_decay.setSliderStyle (juce::Slider::LinearBarVertical);
    membrane_decay.setRange (0.1, 2.0, 0.1);
    membrane_decay.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    membrane_decay.setPopupDisplayEnabled (true, false, this);
    membrane_decay.setTextValueSuffix (" Membrane Pitch");
    membrane_decay.setValue(1.0);
    addAndMakeVisible (&membrane_decay);
    membrane_decay.addListener(this);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::antiquewhite);
    g.setFont (15.0f);

    g.drawFittedText ("Pitch", 0, 0, getWidth(), 30, juce::Justification::centred, 1);
    g.drawFittedText ("Decay", 40, 0, getWidth(), 30, juce::Justification::centred, 1);
}

void AudioPluginAudioProcessorEditor::resized()
{
    renderView.setBounds(120, 30, getWidth() - 140, getHeight() - 60);
    membrane_pitch.setBounds (40, 30, 20, getHeight() - 60);
    membrane_decay.setBounds (80, 30, 20, getHeight() - 60);
}

void AudioPluginAudioProcessorEditor::sliderValueChanged(juce::Slider* slider) 
{
    this->processorRef.membrane_pitch = membrane_pitch.getValue();
    this->processorRef.membrane_decay = membrane_decay.getValue();
}
