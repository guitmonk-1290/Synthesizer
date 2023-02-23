# Synthesizer

Description

The sound waves that we hear normally are continuous in nature, i.e., they can be represented by an infinite number of points on the graph. In order to synthesize these sounds, we need to convert these continuous signals to discrete values, as the computer only has fixed-sized memory. To do this, various discrete values are taken which are interpolated to emulate the continuous sound waves. These discrete values are then fed into the sound card of the device for playback.

Prerequisites
----------------------------------------------------------------------------------------------

One has to install python C++ 11 
Other required libraries are:
    winmm.lib
    Windows.h

Installation
----------------------------------------------------------------------------------------------

After pulling the code, run the main.cpp source file.


Modules and Functions Used:
----------------------------------------------------------------------------------------------

  •	__sound_Interface__ -> This module spawns a thread in the background which is used to communicate with our sound card. The user can supply a custom function to this class for generating waveform data and passing it to this thread.

  •	`sound_Interface(wstring sOutputDevice, unsigned int nSampleRate = 44100, unsigned int nChannels = 1, unsigned int nBlocks = 8, unsigned int nBlockSamples = 512)` -> Creates the sound machine


  •	`static vector<wstring> Enumerate()` –> Get all the output playback devices

  •	`void SetUserFunction(double(*func)(int, double))` –> Set the value-generating function to the one supplied by the user

  •	`void MainThread()` –> Responds to the requests by the sound card to fill blocks

  •	`double makeNoice(int nChannel, double dTime)` –> Custom function for generating waveform data


  •	`double osc(double dHertz, double dTime, int nType, double dLFOHertz = 0.0, double dLFOAmplitude = 0.0)` –> Generates periodic values for various forms of waves including sin waves, square waves, saw-tooth and noise. It is used by the makeNoice function.
