/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CranulatorAudioProcessorEditor::CranulatorAudioProcessorEditor (CranulatorAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),thumbnailCache(5),
thumbnail(512, formatManager, thumbnailCache),
midiKeyboard(p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    setAlwaysOnTop(true);
    setVisible(true);
    addToDesktop(juce::ComponentPeer::StyleFlags::windowHasCloseButton );
    setResizable(true, true);
    positionSlider = new ParameterSlider (*p.position);
    addAndMakeVisible( positionSlider );
    positionSlider->setSliderStyle(juce::Slider::LinearBar);
    positionSlider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);// style, is_readonly, textEntryWidth, TextEntryHeight
    
    addAndMakeVisible(randPosSlider = new ParameterSlider(*p.randPos));
    randPosSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    randPosSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(randPosLabel);
    randPosLabel.setText("randpos", juce::dontSendNotification);
    
    addAndMakeVisible(durationSlider = new ParameterSlider(*p.duration));
    durationSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    durationSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(durationLabel);
    durationLabel.setText("size", juce::dontSendNotification);
    
    addAndMakeVisible(randDurSlider = new ParameterSlider(*p.randDur));
    randDurSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    randDurSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(randDurLabel);
    randDurLabel.setText("randsize", juce::dontSendNotification);
    
    addAndMakeVisible(volumeSlider = new ParameterSlider(*p.volume));
    volumeSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    volumeSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(volumeLabel);
    volumeLabel.setText("gain", juce::dontSendNotification);
    
    addAndMakeVisible(randGainSlider = new ParameterSlider(*p.randGain));
    randGainSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    randGainSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(randGainLabel);
    randGainLabel.setText("randgain", juce::dontSendNotification);
    
    addAndMakeVisible(densitySlider = new ParameterSlider(*p.density));
    densitySlider->setNormalisableRange(juce::NormalisableRange<double> (0.001, 10, 0.001, 0.1));
    densitySlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    densitySlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(densityLabel);
    densityLabel.setText("sparse", juce::dontSendNotification);
    
    addAndMakeVisible(randDensSlider = new ParameterSlider(*p.randDens));
    randDensSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    randDensSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(randDensLabel);
    randDensLabel.setText("rand dens", juce::dontSendNotification);
    
    
    addAndMakeVisible(transposeSlider = new ParameterSlider(*p.transpose));
    transposeSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    transposeSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(transposeLabel);
    transposeLabel.setText("trans", juce::dontSendNotification);
    
    addAndMakeVisible(randPitchSlider = new ParameterSlider(*p.randPitch));
    randPitchSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    randPitchSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(randPitchLabel);
    randPitchLabel.setText("randpitch", juce::dontSendNotification);
    
    addAndMakeVisible(reverseButton = new ParameterButton(*p.reverse));
    reverseButton->setButtonText("rev");
    reverseButton->setToggleable(true);
    reverseButton->setTriggeredOnMouseDown(true);
    reverseButton->setColour(juce::TextButton::buttonColourId, getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    reverseButton->setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    reverseButton->setColour(juce::TextButton::textColourOffId, juce::Colours::darkgrey);
    reverseButton->onClick = [this] { buttonClicked(reverseButton); };
    
    addAndMakeVisible(randRevSlider = new ParameterSlider(*p.randRev));
    randRevSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    randRevSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(randRevLabel);
    randRevLabel.setText("randrev", juce::dontSendNotification);
    
    addAndMakeVisible(blendSlider = new ParameterSlider(*p.blend));
    blendSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    blendSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(blendLabel);
    blendLabel.setText("blend", juce::dontSendNotification);
    
    addAndMakeVisible(envAttackSlider = new ParameterSlider(*p.envAttack));
    envAttackSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    envAttackSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 15);
    addAndMakeVisible(envAttackLabel);
    envAttackLabel.setText("attack", juce::dontSendNotification);
    
    
    addAndMakeVisible(envReleaseSlider = new ParameterSlider(*p.envRelease));
    envReleaseSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    envReleaseSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 15);
    addAndMakeVisible(envReleaseLabel);
    envReleaseLabel.setText("release", juce::dontSendNotification);
    
    addAndMakeVisible(envCurveSlider = new ParameterSlider(*p.envCurve));
    envCurveSlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    envCurveSlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 40, 15);
    addAndMakeVisible(envCurveLabel);
    envCurveLabel.setText("curve", juce::dontSendNotification);
    
    formatManager.registerBasicFormats();
    DBG("Call PluginEditor.");
    //keyboardState.addListener (this);
    thumbnail.addChangeListener(this);
    positionSlider->addListener(this);
    envAttackSlider->addListener(this);
    envReleaseSlider->addListener(this);
    envCurveSlider->addListener(this);
    setSize (660, 400);
    
    midiKeyboard.setName("keyboard");
    addAndMakeVisible(midiKeyboard);
    midiKeyboard.setVelocity(1, true);
    

    (p.keyboardState).addListener(this);
}

CranulatorAudioProcessorEditor::~CranulatorAudioProcessorEditor()
{
    delete positionSlider;
    delete randPosSlider;
    delete durationSlider;
    delete randDurSlider;
    delete volumeSlider;
    delete randGainSlider;
    delete densitySlider;
    delete randDensSlider;
    delete transposeSlider;
    delete randPitchSlider;
    delete reverseButton;
    delete randRevSlider;
    delete blendSlider;
    
    delete envAttackSlider;
    delete envReleaseSlider;
    delete envCurveSlider;
    
    
}

//==============================================================================
void CranulatorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);

    juce::Rectangle<int> thumbnailBounds (10, getHeight() - 160, getWidth() - 20, 100);
    if (thumbnail.getNumChannels() == 0){
        
        if (!audioProcessor.filePath.isEmpty()){
            thumbnail.setSource(new juce::FileInputSource(audioProcessor.filePath));
            paintIfFileLoaded(g, thumbnailBounds);
        }else paintIfNoFileLoaded (g, thumbnailBounds);
    }
    else paintIfFileLoaded (g, thumbnailBounds);
    juce::Rectangle<int> envelopeBounds(440, 50, 160, 90);
    paintEnv(g, envelopeBounds);

    
}
void CranulatorAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    const int width = getWidth();
    positionSlider->setBounds(10, 10, width - 20, 30);
    
    durationSlider->setBounds(10,60,50,65);
    durationLabel.setBounds(10, 40, 50, 20);
    randDurSlider->setBounds(10,145,50, 65);
    randDurLabel.setBounds(10, 125, 50, 20);
    
    volumeSlider->setBounds(70,60,50,65);
    volumeLabel.setBounds(70, 40, 50, 20);
    randGainSlider->setBounds(70,145,50, 65);
    randGainLabel.setBounds(70, 125, 50, 20);
    
    densitySlider->setBounds(130, 60, 50, 65);
    densityLabel.setBounds(130, 40, 50, 20);
    randDensSlider->setBounds(130, 145, 50, 65);
    randDensLabel.setBounds(130, 125, 60, 20);
    
    transposeSlider->setBounds(190, 60, 50, 65);
    transposeLabel.setBounds(190, 40, 50, 20);
    randPitchSlider->setBounds(190, 145, 50, 65);
    randPitchLabel.setBounds(190, 125, 60, 20);
    
    randPosSlider->setBounds(250, 60, 50, 65);
    randPosLabel.setBounds(250, 40, 60, 20);
    randRevSlider->setBounds(250, 145, 50, 65);
    randRevLabel.setBounds(250, 125, 60, 20);
    reverseButton->setBounds(width - 70, getHeight() - 75, 50, 20);
    blendSlider->setBounds(370, 60, 50, 65);
    blendLabel.setBounds(370, 40, 50, 20);
    
    envAttackSlider->setBounds(440, 155, 40, 52);
    envAttackLabel.setBounds(440, 140, 40, 15);
    
    envReleaseSlider->setBounds(500, 155, 40, 52);
    envReleaseLabel.setBounds(500, 140, 50, 15);
    
    envCurveSlider->setBounds(560, 155, 40, 52);
    envCurveLabel.setBounds(560, 140, 40, 15);
    
    midiKeyboard.setBounds(10, getHeight() - 50, width - 20, 50);
}
void CranulatorAudioProcessorEditor::paintEnv(juce::Graphics &g, const juce::Rectangle<int> &envBounds){
    
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(envBounds);
    float a = *audioProcessor.envAttack;
    float r = *audioProcessor.envRelease;
    float c = *audioProcessor.envCurve;
    g.setColour(juce::Colours::white);
    //juce::Path p;
    int envWidth = envBounds.getWidth();
    float gain;
    int envBotton = envBounds.getBottom();
    int envLeft = envBounds.getX();
    int envHeight = envBounds.getHeight();
    int val = 0;
    int prevX,prevY,currentX,currentY;

    for (int i = 0; i < envWidth; i++){
        
        //calculate gain
        gain = envelope(i, envWidth, a, r, c);
        val = (float)gain * envHeight;
        if(i == 0){
            //p.startNewSubPath(envLeft, envBotton);
            prevX = envLeft;
            prevY = envBotton;
        }else {
            //p.lineTo(envLeft + i, envBotton - val);
            currentX = envLeft + i;
            currentY = envBotton - val;
            g.drawLine(prevX, prevY, currentX, currentY);
            prevX = currentX;
            prevY = currentY;
        }
    }
    
}
float CranulatorAudioProcessorEditor::envelope(int i, int length, float a, float r, float c){
    float frac = (float) i/length;
    float g = 0;
    float r0 = 1 - r;
    if(frac < a){
        g = frac / a;
        if(std::abs(c) > 0.01){
            return (1.0f - exp(g * c)) / (1.0f - exp(c));
        }else return g;
    }else if(frac > r0){
        g = (1 - frac)/r;
        if(std::abs(c) > 0.01){
            return (1.0f - exp(g * c)) / (1.0f - exp(c));
        }else return g;
    }
    return 1.0;
}
bool CranulatorAudioProcessorEditor::isInterestedInFileDrag(const juce::StringArray & files){
    for(auto file : files){
        if (file.contains(".wav") || file.contains(".aiff") || file.contains(".aif")) return true;
    }
    return false;
}
void CranulatorAudioProcessorEditor::filesDropped(const juce::StringArray & files, int x, int y){
    for (auto file: files){
        if(isInterestedInFileDrag(files)) {
            audioProcessor.loadFile(file);
            thumbnail.setSource(new juce::FileInputSource(file));
        }
    }
}
void CranulatorAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster *source){
    if (source == &thumbnail) repaint();
    
}
void CranulatorAudioProcessorEditor::paintIfNoFileLoaded(juce::Graphics &g, const juce::Rectangle<int> &thumbnailBounds){
    
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(thumbnailBounds);
    
    g.setColour(juce::Colours::white);
    g.drawFittedText("No Sample Loaded, drop wav/aif here...", thumbnailBounds, juce::Justification::centred, 1);
}
void CranulatorAudioProcessorEditor::paintIfFileLoaded(juce::Graphics &g, const juce::Rectangle<int> &thumbnailBounds){
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(thumbnailBounds);
    g.setColour(juce::Colours::white);
    thumbnail.drawChannels(g, thumbnailBounds, 0.0, thumbnail.getTotalLength(), 1.0f);
    g.setColour(juce::Colours::green);
    float audioPosition = (float) positionSlider->getValue();
    auto drawPosition = (audioPosition * thumbnailBounds.getWidth()) + thumbnailBounds.getX();
    g.drawLine(drawPosition, (float)thumbnailBounds.getY(), drawPosition, (float)thumbnailBounds.getBottom(), 1.0f);
    
    
}

void CranulatorAudioProcessorEditor::buttonClicked (juce::Button* button){
    if (button == reverseButton) click_reverse();
}
void CranulatorAudioProcessorEditor::click_reverse(){
    if(reverseButton->getToggleState()){
        reverseButton->setToggleState(false, juce::dontSendNotification);
        * (audioProcessor.reverse) = false;
    }else{
        reverseButton->setToggleState(true, juce::dontSendNotification);
        * (audioProcessor.reverse) = true;
    }
}

void CranulatorAudioProcessorEditor::handleNoteOn(juce::MidiKeyboardState *source, int midiChannel, int midiNoteNumber, float velocity){
    //std::cout << midiNoteNumber << " on, vel" << (int) (velocity * 127.0f) << std::endl;
    audioProcessor.midiNotes[midiNoteNumber] = (int) (velocity * 127.0f);
    audioProcessor.noteOn = true;
}
void CranulatorAudioProcessorEditor::handleNoteOff(juce::MidiKeyboardState *source, int midiChannel, int midiNoteNumber, float velocity){
    //std::cout << midiNoteNumber << " off" <<  std::endl;
    audioProcessor.midiNotes[midiNoteNumber] = 0;
    //audioProcessor.noteOn = false;
}
