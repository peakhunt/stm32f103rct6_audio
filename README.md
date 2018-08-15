# stm32f103rct6_audio

!!! work still in progress at very early stage!!!

This is a simple audio processor based on stm32f1 as my hobby project.

## Current Architecture
![Architecture](doc/audio_flow.png "Audio Flow")
![Hardware Block](doc/hw_arch.png "hardware block")

## Current Status
15th, August, 2018

All right. With oscilloscope at work, I was able to verify Input/Output.
In the picture below, yellow is ADC input and green is DAC output.
![Measure](doc/oscilloscope.jpg "oscilloscope")

11th, August, 2018.

Since I don't have an oscilloscope, I couldn't precisely verify all the input/output graphs and their timings.
But with test meter and potentiometer, I was able to verify that the output voltage matches the input voltage.
The next phase is to design audio I/O circuit and apply DSP.
