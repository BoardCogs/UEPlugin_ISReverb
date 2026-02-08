# UEPlugin_ISReverb

This Unreal Plugin aims at graphically representing reverb in a room by using the Image Sources method.

## Description

Given the surfaces of a room and the positons of source and listener, sound reflectons are represented by Image Sources (ISs).
ISs are clones of the original source that simulate the original sound being heard by the listener from different directions because of reverberation. Specifically, they used to compute the paths followed by sound rays inside the room. Sound rays, in turn, serve to approximate with acceptable accuracy the behaviour of soundwaves.

The focus of this project is the generation of ISs and the validation of reflection paths for their sound rays, with special attention to implementing different opmizations to drastically cut time costs and achieve real-time computations. The simulation aslo allows to dinamically enable or disable different optimizations independently, as well as multithreading, for benchmarking.

For the full process and algorithms implemented in this simulation, refer to the [official documentation](/ISReverbDocumentation.pdf) inside this folder.
Please note that the documentation describes the method theoretically, but may still contain references to the Unity Game Engine, instead of the Unreal Engine. This is because this plugin is a full porting of a [previous Unity project](https://github.com/BoardCogs/ISReverb), adding multithreading.

## Getting Started

### Dependencies

Running the project to test the plugin requires installing the Epic Games Launcher and Unreal Engine 5.6.

### Executing program

Download this folder and in the Epic Games Launcher navigate to Unreal Engine > Library, then add it as a project from disk if it is not automatically recognised.
If not already installed, the launcher should automatically download the correct editor.

Upon opening the Unreal project, you will find the default level already loaded, complete with a shoebox room to showcase the plugin.

After running the level, in the *World Outliner* panel, under *Sources*, click on the *TestSource* actor.
The *Details* panel will show all actor properties: in the properties derived from the *Source* class, different fields allow to personalize and launch Image Sources generation, as well as debug and view all sound rays and Image Sources. Just hover on a field to see its tooltip.

In the the level, press F8 to freely move the camera around without moving the character. While holding the right mouse button, use the WASD buttons to move and move your mouse to look around.

### Experimenting with the layout

Freely move the *TestSource* and *TestListener* actors, then launch IS generation to try different configurations, or change the position and orientation of the room's surfaces (surfaces already are and always should be children of the *TestRoom* actor, more can be added if needed) to create more complex rooms.
Be careful to extend the room's collision box (a component of the *TestRoom* actor) so that it always covers the entire volume of the room.

### Comparing results

Refer to the *Output Log* panel to see a full review of the final results of each of the two phases of the algorithm and for benchmarking.
To the bottom of the log, a full report is written after the algorithm finishes, containing the number of generated ISs, the ISs cut by optimizations, the number of valid sound rays detected and, last but not least, the execution time required by the two phases of the algorithm.

## Authors

[Gianmaria Forte](www.linkedin.com/in/gianmaria-forte-278306261)