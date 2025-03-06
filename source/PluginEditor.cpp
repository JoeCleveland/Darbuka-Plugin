#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);

    addAndMakeVisible(renderView);

    juce::Colour sliderColor = juce::Colour::fromRGB(127, 40, 0);

    filter_cut.setSliderStyle (juce::Slider::RotaryHorizontalDrag);
    filter_cut.setRange (-10000.0, 10000.0, 1.0);
    filter_cut.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    filter_cut.setPopupDisplayEnabled (true, false, this);
    filter_cut.setTextValueSuffix ("FILTER CUTOFF");
    filter_cut.setValue(400.0);
    filter_cut.setColour(0, juce::Colours::crimson);
    filter_cut.setColour(juce::Slider::ColourIds::trackColourId, sliderColor);
    addAndMakeVisible (&filter_cut);
    filter_cut.addListener(this);

    membrane_decay.setSliderStyle (juce::Slider::RotaryHorizontalDrag);
    membrane_decay.setRange (0.9, 0.999999, 0.001);
    membrane_decay.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    membrane_decay.setPopupDisplayEnabled (true, false, this);
    membrane_decay.setTextValueSuffix ("DECAY");
    membrane_decay.setValue(0.999999);
    membrane_decay.setColour(juce::Slider::ColourIds::trackColourId, sliderColor);
    addAndMakeVisible (&membrane_decay);
    membrane_decay.addListener(this);

    membrane_youngs_mod.setSliderStyle(juce::Slider::RotaryHorizontalDrag);
    membrane_youngs_mod.setRange (0.5, 32.0, 0.1);
    membrane_youngs_mod.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    membrane_youngs_mod.setPopupDisplayEnabled (true, false, this);
    membrane_youngs_mod.setTextValueSuffix ("YOUNGS MODULUS");
    membrane_youngs_mod.setValue(12.0);
    membrane_youngs_mod.setColour(juce::Slider::ColourIds::trackColourId, sliderColor);
    addAndMakeVisible (&membrane_youngs_mod);
    membrane_youngs_mod.addListener(this);

    membrane_moment_inert.setSliderStyle(juce::Slider::RotaryHorizontalDrag);
    membrane_moment_inert.setRange (0.5, 32.0, 0.1);
    membrane_moment_inert.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    membrane_moment_inert.setPopupDisplayEnabled (true, false, this);
    membrane_moment_inert.setTextValueSuffix ("MOMENT OF INERTIA");
    membrane_moment_inert.setValue(12.0);
    membrane_moment_inert.setColour(juce::Slider::ColourIds::trackColourId, sliderColor);
    addAndMakeVisible (&membrane_moment_inert);
    membrane_moment_inert.addListener(this);

    membrane_mass_density.setSliderStyle(juce::Slider::RotaryHorizontalDrag);
    membrane_mass_density.setRange (0.001, 2.0, 0.001);
    membrane_mass_density.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    membrane_mass_density.setPopupDisplayEnabled (true, false, this);
    membrane_mass_density.setTextValueSuffix ("MASS DENSITY");
    membrane_mass_density.setValue(0.1);
    membrane_mass_density.setColour(juce::Slider::ColourIds::trackColourId, sliderColor);
    addAndMakeVisible (&membrane_mass_density);
    membrane_mass_density.addListener(this);

    membrane_spoke_length.setSliderStyle(juce::Slider::RotaryHorizontalDrag);
    membrane_spoke_length.setRange (0.05, 3.0, 0.001);
    membrane_spoke_length.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    membrane_spoke_length.setPopupDisplayEnabled (true, false, this);
    membrane_spoke_length.setTextValueSuffix ("RADIUS");
    membrane_spoke_length.setValue(0.2);
    membrane_spoke_length.setColour(juce::Slider::ColourIds::trackColourId, sliderColor);
    addAndMakeVisible (&membrane_spoke_length);
    membrane_spoke_length.addListener(this);

    membrane_crust_ratio.setSliderStyle(juce::Slider::RotaryHorizontalDrag);
    membrane_crust_ratio.setRange (0.0, 1.0, 0.01);
    membrane_crust_ratio.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    membrane_crust_ratio.setPopupDisplayEnabled (true, false, this);
    membrane_crust_ratio.setTextValueSuffix ("CRUST RATIO");
    membrane_crust_ratio.setValue(0.0);
    membrane_crust_ratio.setColour(juce::Slider::ColourIds::trackColourId, sliderColor);
    addAndMakeVisible (&membrane_crust_ratio);
    membrane_crust_ratio.addListener(this);

    bending_force.setSliderStyle(juce::Slider::RotaryHorizontalDrag);
    bending_force.setRange (0.0, 3.0, 0.01);
    bending_force.setTextBoxStyle (juce::Slider::NoTextBox, false, 90, 0);
    bending_force.setPopupDisplayEnabled (true, false, this);
    bending_force.setTextValueSuffix ("BENDING FORCE");
    bending_force.setValue(0.0);
    bending_force.setColour(juce::Slider::ColourIds::trackColourId, sliderColor);
    addAndMakeVisible (&bending_force);
    bending_force.addListener(this);

    setSize (800, 600);

    startTimer(100);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colour::fromRGB(0, 15, 25));

    g.setColour (juce::Colour::fromRGB(115, 115, 110));
    g.setFont (15.0f);

    g.drawFittedText ("Filter", 40, 80, 40, 15, juce::Justification::centred, 1);
    g.drawFittedText ("Decay", 80, 80, 40, 15, juce::Justification::centred, 1);
    g.drawFittedText ("Youngs", 120, 80, 40, 15, juce::Justification::centred, 1);
    g.drawFittedText ("Inertia", 160, 80, 40, 15, juce::Justification::centred, 1);
    g.drawFittedText ("Density", 200, 80, 40, 15, juce::Justification::centred, 1);
    g.drawFittedText ("Size", 240, 80, 40, 15, juce::Justification::centred, 1);
    g.drawFittedText ("Ratio", 280, 80, 40, 15, juce::Justification::centred, 1);
}

void AudioPluginAudioProcessorEditor::resized()
{
    renderView.setBounds(320, 30, getWidth() - 140, getHeight() - 60);

    filter_cut.setBounds (40, 30, 40, 40);
    membrane_decay.setBounds (80, 30, 40, 40);
    membrane_youngs_mod.setBounds (120, 30, 40, 40);
    membrane_moment_inert.setBounds (160, 30, 40, 40);
    membrane_mass_density.setBounds (200, 30, 40, 40);
    membrane_spoke_length.setBounds (240, 30, 40, 40);
    membrane_crust_ratio.setBounds (280, 30, 40, 40);

    bending_force.setBounds (40, 120, 40, 40);
}

void AudioPluginAudioProcessorEditor::sliderValueChanged(juce::Slider* slider) 
{
    this->processorRef.rt_params.cutoff = filter_cut.getValue();
    this->processorRef.rt_params.decay = membrane_decay.getValue();

    this->processorRef.ol_params.youngs_mod = membrane_youngs_mod.getValue();
    this->processorRef.ol_params.moment_inert = membrane_moment_inert.getValue();
    this->processorRef.ol_params.mass_density = membrane_mass_density.getValue();
    this->processorRef.ol_params.spoke_length = membrane_spoke_length.getValue();
    this->processorRef.ol_params.crust_ratio = membrane_crust_ratio.getValue();

    this->processorRef.ol_params.pressing_force = bending_force.getValue();
}

void AudioPluginAudioProcessorEditor::timerCallback() {
    if(this->processorRef.force_pattern.size() > 0 && 
       this->processorRef.force_pattern.minCoeff() > 0) {
        this->renderView.force_pattern = this->processorRef.force_pattern;
        this->processorRef.force_pattern = Eigen::ArrayXd::Zero(0);
    }

    if(this->processorRef.curr_geom.points.size() > 0) {
        this->renderView.curr_geom = this->processorRef.curr_geom;
    }
}
