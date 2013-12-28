This repository contains the source code for the ARM side libraries used on Raspberry Pi
and RaspiBGS - some beginner example of simple background substraction which use GPU.

It uses modified version of raspivid and modified version of graphics.cpp from wibble82.

Some parts of code are in C and some in C++, sorry.

How to build

	cmake .
	make
	

For configuration of some parameters is used raspibgs.cfg file, but for basic tests is not needed to modify it, in this case are
used default parameters.

Image processing use vertex and fragment shaders, which are placed in glsl/ directory, but for run raspibgs must be placed
in diredtory where is raspibgs executed.

example of use:

	raspibgs -t 0 -v -o /tmp/md_%d.h264

Behaviour should be:

check scene motion, if is something detected then is for 30seconds saved H264 video.
default resoultion is 1920x1080 for H264 video, for image processing only 320x240.
	
when is sensitivity too high/low - try to modify raspibgs.cfg file, interesting parameters are
	
	[BGS]
	ObjectSizeX=2
	ObjectSizeY=2
	Threshold=4
		
Note: file raspibgs.cfg must be in directory where is raspibgs executed!

For built without OpenCV - modify host_applications/linux/apps/raspicam/CMakeLists.txt and remove following line: add_definitions(-D_D_USE_OPENCV)
In this case doesn't work motion detection!

Most time consuming part is taken data from GPU and prepare them for OpenCV which then locate object on scene, this can be defintely also
made by GPU but it will need more research.



