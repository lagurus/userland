This repository contains the source code for the ARM side libraries used on Raspberry Pi
and RaspiBGS - simple example of basic background subtraction which uses GPU.

It uses modified version of raspivid and modified version of graphics.cpp from wibble82.

Some parts of code are in C and some in C++.

How to build:

	git clone https://github.com/lagurus/userland.git raspibgs
	cd raspibgs
	cmake .
	make


When build is finished you can find raspibgs in ./build/bin/ directory.

Some parameters can be configured only in raspibgs.cfg config file, but for basic tests it is not needed to modify anything (predefined parameters are used).

Image processing uses vertex and fragment shaders, which are placed in glsl/ directory, but raspibgs needs them in directory where is raspibgs executed. You can use script copyglsl.sh which copy *.glsl files into ./build/bin/ directory.

	./copyglsl.sh


example of use:

	./build/bin/raspibgs -t 0 -v -o /tmp/alarm_%d.h264

Behaviour should be: Check movements and if something is detected then save 30seconds of H264 video.

Default resolution is 1920x1080 for H264 video, for image processing 320x240.
	
When sensitivity is too high/low - try to modify raspibgs.cfg file, interesting parameters can be
	
	[BGS]
	ObjectSizeX=3
	ObjectSizeY=3
	Threshold=9
		
Note: file raspibgs.cfg must be in directory where is raspibgs executed!

For build without OpenCV - modify file "host_applications/linux/apps/raspicam/CMakeLists.txt" and remove following line: add_definitions(-D_D_USE_OPENCV)
But in this case motion detection doesn't work!

The most time consuming part is taking data from GPU and preparing them for OpenCV which locates objects in the scene. This can be made by GPU too - in some later version.



