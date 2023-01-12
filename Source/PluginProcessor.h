/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/

class Grain
{
public:
    const long long int onset;
    const int length;
    const int startPos;
    
    
    const float envAttack, envAttackRecip;
    const float envRelease, envReleaseRecip;
    const float envCurve;
    const float lengthRecip;
    
    
    const float rate;
    const float amp;
    const bool rev;
    const int grainLengthInSample;
    
    
    Grain(): onset(0),length(1000), startPos(0), envAttack(0.3), envAttackRecip(1/envAttack), envRelease(0.7), envReleaseRecip(1/(1-envRelease)),envCurve(0.0), lengthRecip(1/length), rate(1.0), amp(1.0), rev(false), grainLengthInSample(int(length/rate))
    {}
    ~Grain(){}
    Grain(long long int onset,int length,int startPos,float envAttack=0.3, float envR=0.3, float envCurve = 0.0, float rate=1.0, float amp = 1.0, bool reverse = false): onset(onset), length(length), startPos(startPos), envAttack(envAttack), envAttackRecip(1/envAttack), envRelease(1-envR), envReleaseRecip(1/envR), envCurve(envCurve), lengthRecip(1/(float)length), rate(rate),amp(amp), rev(reverse), grainLengthInSample(int(length/rate))
    {
        
    }
    float envelope(int time){
        
        //curve method
        // if c != 0 => gain = (1 - e^fc) / (1 - e^c)
        // f is fraction of position in curve (from the lowest point)
        float gain = 0;
        float envPos = (float)(time - onset) * lengthRecip;
        if(envPos <= envAttack){
            gain = envPos * envAttackRecip;
            if(std::abs(envCurve) > 0.001 ) return (1.0 - exp(gain * envCurve)) / (1.0 - exp(envCurve));
            return gain;
        }else if (envPos >= envRelease){
            gain = (1-envPos) * envReleaseRecip;
            if (std::abs(envCurve) > 0.001) return (1.0 - exp(gain * envCurve)) / (1.0 - exp(envCurve));
            return gain;
        }else return 1.0;
    }
    inline float linearInterp(float x, float y0, float y1) {return x * y0 + (1-x) * y1;}
    void process (juce::AudioSampleBuffer& currentBlock, juce::AudioSampleBuffer& fileBuffer, int numChannels, int blockNumSamples, int fileNumSamples, long long int time)
    {
        for(int channel=0; channel<numChannels; ++channel){
            const float gain = (int) envelope(time) ;

            float* channelData = currentBlock.getWritePointer(channel);
            const float* fileData = fileBuffer.getReadPointer(channel%fileBuffer.getNumChannels());
            float diff = (int) (time - onset);
            const float position = (float)diff * rate;
            
            const int iPosition = (int) std::ceil(position);
            const float fracPos = iPosition - position;
            const int rev_p = rev ? -1 : 1;
//            const float pos = (float) rev_p * (time - onset) * rate ;
//            const int prevPos = (int) pos;
//            const int currentPos = prevPos + rev_p;

//            const float frac = rev_p * (pos - prevPos);
//            float currentSample = fileData[currentPos % length];
//            const float prevSample = fileData[prevPos % length];
//            currentSample = linearInterp(frac, prevSample, currentSample) * gain * amp;
            const int readPos = rev_p * iPosition + startPos + fileNumSamples;
            float currentSample = fileData[readPos % fileNumSamples];
//            float a = fileData[(readPos - 3) % fileNumSamples];
//            float b = fileData[(readPos - 2) % fileNumSamples];
//            float c = fileData[(readPos - 1) % fileNumSamples];
            //currentSample = interpolation(fracPos , a, b, c, currentSample);
            float prevSample = fileData[(readPos - rev_p) % fileNumSamples];
            currentSample = linearInterp(fracPos, prevSample, currentSample);
            currentSample = currentSample * gain * amp;
            channelData[time % blockNumSamples] += currentSample;
        }
    }
};
class ReferenceCountedBuffer : public juce::ReferenceCountedObject
{
public:
    ReferenceCountedBuffer (const juce::String& buffername, int numChannels, int numSamples):
    name(buffername),buffer(numChannels, numSamples)
    {}
    ~ReferenceCountedBuffer()
    {}
    juce::AudioSampleBuffer* get(){
        return &buffer;
    }
    typedef  juce::ReferenceCountedObjectPtr<ReferenceCountedBuffer> Ptr;
private:
    juce::String name;
    juce::AudioSampleBuffer buffer;
};
class CranulatorAudioProcessor  : public juce::AudioProcessor, public juce::Thread
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif

{
public:
    //==============================================================================
    CranulatorAudioProcessor();
    ~CranulatorAudioProcessor() override;
//    int noteOnValue, velValue;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processMidi (juce::MidiBuffer& midiMessage, int numSamples);

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    juce::AudioProcessorEditor* audioProcessorEditor;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override/* { return 1;}*/;
    int getCurrentProgram() override/* { return 0;}*/;
    void setCurrentProgram (int index) override/*{}*/;
    const juce::String getProgramName (int index) override/* { return juce::String(); }*/;
    void changeProgramName (int index, const juce::String& newName) override /*{}*/;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    
    void run() override;
    
    
    double fs;
    long long int time;
    long long int nextGrainOnset;
    
    //grains
    juce::Array<Grain> grains;
    int midiNotes[128] = {0};
    
    // Parameters
    juce::AudioParameterFloat* position;
    juce::AudioParameterFloat* randPos;
    juce::AudioParameterFloat* duration;
    juce::AudioParameterFloat* randDur;
    juce::AudioParameterFloat* volume;
    juce::AudioParameterFloat* randGain;
    juce::AudioParameterFloat* density;
    juce::AudioParameterFloat* randDens;
    juce::AudioParameterBool* reverse;
    juce::AudioParameterFloat* randRev;
    
    juce::AudioParameterFloat* randPitch;
    juce::AudioParameterFloat* blend;
    juce::AudioParameterInt* transpose;
    juce::AudioParameterFloat * envAttack;
    juce::AudioParameterFloat* envRelease;
    juce::AudioParameterFloat* envCurve;
    
    
    
    //Utility
    int wrap2int(int val, const int min, const int max);
    float clip(float sample, const float min, const float max);
    bool noteOn;
    juce::MidiKeyboardState keyboardState;
    
    //LoadFile
    void loadFile(const juce::String & path);
    bool checkRestorePath();
    juce::String restorePath;
    juce::String filePath;
    juce::ReferenceCountedObjectPtr<ReferenceCountedBuffer> fileBuffer;
    
private:
    inline float linearInterp(float x, float y0, float y1) {return x * y1 + (1-x) * y0;}
    juce::AudioFormatManager formatManager;
    juce::AudioDeviceManager deviceManager;
    float currentPos;
    float rate;
    float binsPerOctave;
    juce::File fileToPlay;
    int schedDelay;
    juce::FileLogger* crLog = juce::FileLogger::createDefaultAppLogger("CRN", "CRN.log", "CRN LOG:", 256*1024);
    

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CranulatorAudioProcessor)
};
