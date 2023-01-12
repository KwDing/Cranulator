/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CranulatorAudioProcessor::CranulatorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
, Thread("schedulingThread"),position(nullptr),duration(nullptr),volume(nullptr)

{
    addParameter(position = new juce::AudioParameterFloat(juce::ParameterID{"POS", 1}, "Position",
                                                          0.0001f, 1.0f, 0.5f));
    addParameter(duration = new juce::AudioParameterFloat(juce::ParameterID{"DUR", 1}, "Duration",
                                                          juce::NormalisableRange<float>
                                                          (0.001, 4, 0.001, 0.3), 0.7f));
    addParameter(volume = new juce::AudioParameterFloat(juce::ParameterID{"VOL", 1}, "Volume",
                                                        juce::NormalisableRange<float>
                                                        (0.001, 1.0, 0.001, 0.7), 0.7f));
    addParameter(density = new juce::AudioParameterFloat(juce::ParameterID{"DEN", 1}, "Density",
                                                         juce::NormalisableRange<float>
                                                         (0.01, 10, 0.001, 0.9), 0.4f));
    addParameter(reverse = new juce::AudioParameterBool(juce::ParameterID{"REV", 1}, "Reverse", false));
    addParameter(randPos = new juce::AudioParameterFloat(juce::ParameterID{"RAND_POS", 1}, "rand_pos",
                                                         0.0f, 1.0f, 0.1f));
    addParameter(randPitch = new juce::AudioParameterFloat(juce::ParameterID{"RAND_PITCH", 1}, "rand_pitch",
                                                         0.0f, 1.0f, 0.05f));
    addParameter(randDur = new juce::AudioParameterFloat(juce::ParameterID{"RAND_DUR", 1},
                                                         "rand_dur", 0.0f, 1.0f, 0.05f));
    addParameter(randGain = new juce::AudioParameterFloat(juce::ParameterID{"RAND_GAIN", 1},
                                                          "rand_gain", 0.0f, 1.0f, 0.1f));
    addParameter(randDens = new juce::AudioParameterFloat(juce::ParameterID{"RAND_DENS", 1},
                                                          "rand_dens", 0.0f, 1.0f, 0.1f));
    addParameter(transpose = new juce::AudioParameterInt(juce::ParameterID{"TRANSPOSE", 1}, "trans", -24, 24, 0));
    addParameter(blend = new juce::AudioParameterFloat(juce::ParameterID{"BLEND", 1}, "blend", 0.0f, 1.0f, 1.0f));
    addParameter(envAttack = new juce::AudioParameterFloat(juce::ParameterID{"ATTACK", 1}, "attack", 0.0f, 0.5f, 0.3f));
    addParameter(envRelease = new juce::AudioParameterFloat(juce::ParameterID{"RELEASE", 1}, "release", 0.0f, 0.5f, 0.3f));
    addParameter(envCurve = new juce::AudioParameterFloat(juce::ParameterID{"CURVE", 1}, "curve", -5.0f, 5.0f, 0.0f));
    addParameter(randRev = new juce::AudioParameterFloat(juce::ParameterID{"RAND_REV", 1}, "rand_rev", 0.0f, 1.0f, 1.0f));
    time = 0;
    schedDelay = 700;
    formatManager.registerBasicFormats();
    binsPerOctave = 12.0f;
    noteOn = false;
    keyboardState.reset();
    juce::Logger::setCurrentLogger(crLog);
    startThread();
}

CranulatorAudioProcessor::~CranulatorAudioProcessor()
{
    stopThread(5000);
    juce::Logger::setCurrentLogger(nullptr);
    delete crLog;
}

//==============================================================================
const juce::String CranulatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CranulatorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CranulatorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CranulatorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CranulatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CranulatorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CranulatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CranulatorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CranulatorAudioProcessor::getProgramName (int index)
{
    return {};
}

void CranulatorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CranulatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    fs = sampleRate;
}

void CranulatorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CranulatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void CranulatorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    //juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamplesInBlock = buffer.getNumSamples();
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
   
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; i++)
        buffer.clear (i, 0, numSamplesInBlock);
    processMidi(midiMessages, numSamplesInBlock);
    
    
    juce::ReferenceCountedObjectPtr<ReferenceCountedBuffer> retainedBuffer (fileBuffer);
    if (retainedBuffer == nullptr) return;
    
    juce::AudioSampleBuffer* currentBuffer = retainedBuffer->get();
    
    
    const juce::Array<Grain> stack = grains;
    const int numSamplesInFile  = currentBuffer->getNumSamples();
    bool checkNoteOn = false;
    for(int i = 0; i < 128; i++){
        if (midiNotes[i] > 0) checkNoteOn = true;
    }
    if (!checkNoteOn) noteOn = false;
    for( int i = 0; i < numSamplesInBlock; i++){
        
        for (int g = 0; g < stack.size(); g++){
            if(stack[g].onset < time){
                if(time < (stack[g].onset + stack[g].length)){
                    stack[g].process(buffer, *currentBuffer,  buffer.getNumChannels(), numSamplesInBlock, numSamplesInFile, time);
                }
            }
        }
        //check BAD val
        rate = pow(2, *transpose / binsPerOctave);
        float rev_p = *reverse? -1 : 1;

        if (currentPos < 0) currentPos += (float)numSamplesInFile;
        int pos1 = (int) currentPos;
        int pos2 = currentPos + (int)rev_p;
        float frac = rev_p* (currentPos - (float) pos1);
        
        for(int c = 0; c < buffer.getNumChannels(); c++){
            float* channelData = buffer.getWritePointer(c);
            float currentSample = channelData[i];
//            for(int g = 0; g < stack.size(); g++){
//                if(currentSample > 1.0 || currentSample < -1.0){
//                    std::cout<<"badsample " << currentSample << ", dump "<<stack.size() << "grains"<<std::endl;
//                }
//            }
            float dry = 0;
            if(noteOn){
                const float* fileData = currentBuffer->getReadPointer(c%currentBuffer->getNumChannels());
                float sample1 = fileData[pos1 % numSamplesInFile];
                float sample2 = fileData[pos2 % numSamplesInFile];
                dry = linearInterp(frac, sample1, sample2);
            }
            currentSample *= *blend;
            currentSample += (1 - *blend) * dry;
            channelData[i] = clip(currentSample, -1.0f, 1.0f);
        }
        if(noteOn){
            //std::cout<< currentPos << " " << rate << " " << *transpose << " " << std::endl;
            currentPos += (float)rev_p * rate;
            if (currentPos > numSamplesInFile) currentPos-= (float)numSamplesInFile;
        }else{
            currentPos =  (*position) * (float)numSamplesInFile;
        }
        
        
        time++;
    }
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
}

void CranulatorAudioProcessor::processMidi (juce::MidiBuffer& midiMessage, int numSamples){
    juce::MidiMessage m;
    for(auto meta : midiMessage){
        m = meta.getMessage();
        if(m.isNoteOn()){
            midiNotes[m.getNoteNumber()] = m.getVelocity();
            noteOn = true;
            notify();
        }
        if(m.isNoteOff()) {
            midiNotes[m.getNoteNumber()] = 0;
        }
//        if(m.isAllNotesOff()){
//            for(int i = 0; i < 128; i++)    midiNotes[i] = 0;
//        }
    }
}
bool CranulatorAudioProcessor::checkRestorePath(){
    if(restorePath.isEmpty()) return false;
    juce::File f(restorePath);
    if(f.exists()){
        loadFile(restorePath);
        restorePath = "";
        return true;
    }
    return false;
}

void CranulatorAudioProcessor::loadFile (const juce::String & path){
    juce::File fileToPlay(path);
    DBG(path);
    juce::AudioFormatReader* reader = formatManager.createReaderFor(fileToPlay);
    juce::ReferenceCountedObjectPtr<ReferenceCountedBuffer> newBuffer =
    new ReferenceCountedBuffer(fileToPlay.getFileName(), reader->numChannels,reader->lengthInSamples);
    reader->read(newBuffer->get(), 0, reader->lengthInSamples, 0, true, true); //buffer, startsample in destbuff,numsmple,reader start,useleft,use right
    std::cout << "Read Buffer: " << reader->lengthInSamples << " Samples!\n";
    fileBuffer = newBuffer;
    std::cout << fileBuffer.get() << std::endl;
    filePath = path;
    delete reader;
}
//==============================================================================
bool CranulatorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CranulatorAudioProcessor::createEditor()
{
    audioProcessorEditor = new CranulatorAudioProcessorEditor(*this);
    return audioProcessorEditor;
}

//==============================================================================
void CranulatorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::XmlElement xml("CranulatorSettings");
    for (int i = 0; i < getNumParameters(); i++){
        if (juce::AudioProcessorParameterWithID* p = dynamic_cast<juce::AudioProcessorParameterWithID*> (getParameters().getUnchecked(i)))
        {
            xml.setAttribute (p->paramID, p->getValue());
        }
        xml.setAttribute ("restorePath", filePath);
        copyXmlToBinary (xml, destData);
    }
}

void CranulatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto xmlState = (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr){
        if (xmlState->hasTagName ("CranulatorSettings")){
            for (int i = 0; i < getNumParameters(); ++i)
            {
                if (juce::AudioProcessorParameterWithID* p = dynamic_cast<juce::AudioProcessorParameterWithID*> (getParameters().getUnchecked(i)))
                {
                    p->setValueNotifyingHost ((float) xmlState->getDoubleAttribute (p->paramID, p->getValue()));
                    
                }
            }
            restorePath = xmlState->getStringAttribute("restorePath");
        }
    }
}

void CranulatorAudioProcessor::run(){
    
    while (! threadShouldExit()){
        checkRestorePath();
        
        if(grains.size() > 0){
            for (int i = grains.size() - 1; i >= 0; i--){
                long long int grainEnd = grains[i].onset + grains[i].grainLengthInSample;
                if(grainEnd < time) grains.remove(i);
            }
        }
        
        juce::Array<juce::Array<int>> activeNotes;
        
        for(int i = 0; i < 128; i++){
            if(midiNotes[i] > 0){
                activeNotes.add(juce::Array<int> {i, midiNotes[i]} );
            }
        }
        //add to grains
        if (fileBuffer != nullptr){
            if(activeNotes.size() > 0){
                if (nextGrainOnset == 0) nextGrainOnset = time;
                
                int numSamples = fileBuffer->get()->getNumSamples();
                float midiNote = 60;
                midiNote = activeNotes[juce::Random::getSystemRandom().nextInt(activeNotes.size())][0] - 60 + (*transpose);
                float r = pow (2.0, midiNote / binsPerOctave);
                r *= 0.5 * (0.5 - juce::Random::getSystemRandom().nextFloat()) * (*randPitch) + 1;
                //Duration
                float dur = *duration;
                dur *= 1 + 0.1 * juce::Random::getSystemRandom().nextFloat() * (*randDur);
                int length = dur * fs;
               
                long long int onset = nextGrainOnset + schedDelay;
                //Density
                float dens = *density;
                dens *= 1 + (0.5 - juce::Random::getSystemRandom().nextFloat())* (*randDens);
                
                
               
                
                //Position
                float startPos = (*position + (*randPos) * (juce::Random::getSystemRandom().nextFloat() - 0.5)) * numSamples;
                startPos = wrap2int(startPos, 0, numSamples);

                
                //Amplitude
                float amp = *volume;
                amp *= 1 - juce::Random::getSystemRandom().nextFloat() * (*randGain);
                
                bool R = *reverse;
                if (0.5 * (*randRev + 1.0f) * juce::Random::getSystemRandom().nextFloat() > 0.5){
                    R = !R;
                }
                //randAmp
                nextGrainOnset = onset + (dens * dur * fs);
                grains.add(Grain(onset, length, startPos, *envAttack, *envRelease, *envCurve, r, amp, *reverse));
                double schedError = ((onset - schedDelay) - time) / fs;
                dur += schedError;
                wait(dens * dur * 1000);
            }else{
                nextGrainOnset = 0;
                wait(100);
            }
        }else{
            wait(100);
        }
    }
}

int CranulatorAudioProcessor::wrap2int(int val, const int min, const int max){
    int range = max - min + 1;
    if (val < min)
        val += range * ( (min - val) / range + 1);
    return min + (val - min) % range;
}
float CranulatorAudioProcessor::clip(float sample, const float min, const float max){
    if(sample > max) return max;
    else if(sample < min) return min;
    return sample;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CranulatorAudioProcessor();
}
