# Cranulator
This is a ganular synthesizor built upon JUCE, Cpp in Xcode. 
## GUI
This is the view of the standalone app, vst3 and AU plugins works the same way.
<img width="668" alt="image" src="https://user-images.githubusercontent.com/71970518/212154102-9f33a495-1c35-4b0a-8179-48ab16be0b0f.png">

## Parameters
1. The position slider is on the top, the position and randpos controls the start position of audio and grain
2. Size and randsize controls grain size
3. Sparse and rand dens controls the density. 
4. Audio file could be dragged into the plugin
5. Trans stands for transpose. Trans and controls the frequency of the audio and grains. 
6. Randpitch only switch the frequency of the grains
7. The rev button at the right down corner controls whether the audio or grain is reversed.
8. Randrev stands for the percentage of grains whose playback mode is different from the rev mode.
9. For the envelope curve parameter, it makes the envelope attack release convex when it's below 0, and concave above 0. When curve is zero, the envelope attack/release will be a line.
