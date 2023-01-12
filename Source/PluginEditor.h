/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class ParameterSlider: public juce::Slider, private juce::Timer{
public:
    juce::AudioProcessorParameter& param;
    ParameterSlider(juce::AudioProcessorParameter& p) : juce::Slider(p.getName(256)), param(p){
        setRange(0.0f, 1.0f, 0.0f);
        startTimerHz(60);
        updateSliderPos();
    }
    ParameterSlider(juce::AudioProcessorParameter& p, float min, float max, float range) : juce::Slider(p.getName(256)), param(p){
        setRange(min, max, range);
        startTimerHz(60);
        updateSliderPos();
    }
    bool isDragging = false;
    void startedDragging() override{ param.beginChangeGesture(); isDragging = true;}
    void stoppedDragging() override{ param.endChangeGesture();   isDragging = false;}
    void timerCallback() override{updateSliderPos();}
    void updateSliderPos(){
        const float newVal = param.getValue();
        if (newVal != (float) juce::Slider::getValue()) {Slider::setValue(newVal);}
    }
    void valueChanged() override{
        if(isDragging) param.setValueNotifyingHost( (float) juce::Slider::getValue());
        else {
            param.setValue( (float) juce::Slider::getValue());};
    }
    double getValueFromText(const juce::String& text)override {return param.getValueForText (text);}
    juce::String getTextFromValue(double val) override{return param.getText(float(val), 1024);}
};

class ParameterButton: public juce::TextButton, private juce::Timer{
public:
    juce::AudioProcessorParameter& param;
    ParameterButton(juce::AudioProcessorParameter& p) : juce::TextButton(p.getName(256)), param(p){
        updateButton();
        startTimerHz(60);
    }
    void timerCallback() override{updateButton();}
    void updateButton(){
        const bool newVal = param.getValue();
        if (newVal != juce::TextButton::getToggleState())
            juce::TextButton::setToggleState(newVal,juce::sendNotification);
    }
    void clicked() override { param.setValueNotifyingHost(juce::TextButton::getToggleState()); }
};

class CranulatorAudioProcessorEditor  : public juce::AudioProcessorEditor,
public juce::FileDragAndDropTarget,
private juce::Button::Listener,
private juce::Slider::Listener,
private juce::ChangeListener,
private juce::MidiKeyboardStateListener
{
public:
    CranulatorAudioProcessorEditor (CranulatorAudioProcessor&);
    ~CranulatorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void paintThumbnail (juce::Graphics& g);
    //juce::
    bool isInterestedInFileDrag(const juce::StringArray & files) override;
    void filesDropped(const juce::StringArray & files, int x, int y) override;
    void sliderValueChanged (juce::Slider* slider) override{
        if(slider == positionSlider || randPosSlider){
            repaint();
        }
    };
    void buttonClicked (juce::Button* button) override;
    void click_reverse();
    void changeListenerCallback(juce::ChangeBroadcaster * source) override;
    void paintIfNoFileLoaded (juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);
    void paintIfFileLoaded (juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);
    void paintEnv(juce::Graphics& g, const juce::Rectangle<int>& envBounds);
    void handleNoteOn(juce::MidiKeyboardState * source, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState * source, int midiChannel, int midiNoteNumber, float velocity) override;
    
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    CranulatorAudioProcessor& audioProcessor;
    
    float envelope(int index, int length, float a, float r, float c);
    

    ParameterButton* reverseButton;
    ParameterSlider* randRevSlider;
    juce::Label randRevLabel;
    ParameterSlider* positionSlider;
    ParameterSlider* randPosSlider;
    juce::Label randPosLabel;
    ParameterSlider* durationSlider;
    juce::Label durationLabel;
    ParameterSlider* randDurSlider;
    juce::Label randDurLabel;
    ParameterSlider* volumeSlider;
    juce::Label volumeLabel;
    ParameterSlider* randGainSlider;
    juce::Label randGainLabel;
    ParameterSlider* densitySlider;
    juce::Label densityLabel;
    ParameterSlider* randDensSlider;
    juce::Label randDensLabel;
    ParameterSlider* transposeSlider;
    juce::Label transposeLabel;
    ParameterSlider* randPitchSlider;
    juce::Label randPitchLabel;
    ParameterSlider* blendSlider;
    juce::Label blendLabel;
    ParameterSlider* envAttackSlider;
    juce::Label envAttackLabel;
    ParameterSlider* envReleaseSlider;
    juce::Label envReleaseLabel;
    ParameterSlider* envCurveSlider;
    juce::Label envCurveLabel;

    
    //Thumbnail
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;
    
    //Utilities
    juce::AudioFormatManager formatManager;
    juce::MidiKeyboardComponent midiKeyboard;
    //juce::MidiKeyboardState keyboardState;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CranulatorAudioProcessorEditor)
};
