This repository contains the source code for the ARM side libraries used on Raspberry Pi
and RaspiBGS - simple example of basic background subtraction which uses GPU.

It uses modified version of raspivid and modified version of graphics.cpp from wibble82.

Some parts of code are in C and some in C++.

How to build:

	git clone https://github.com/lagurus/userland.git raspibgs
	cd raspibgs
	cmake .
	make


After successfully build is created raspibgs in ./build/bin/

Some parameters can be configured only in raspibgs.cfg config file, but for basic tests is not needed to modify it. In this case are used default parameters.

Image processing uses vertex and fragment shaders, which are placed in glsl/ directory, but raspibgs need them in directory where is raspibgs executed. You can use script copyglsl.sh which copy *.glsl files into ./build/bin/

	./copyglsl.sh


example of use:

	./build/bin/raspibgs -t 0 -v -o /tmp/alarm_%d.h264

Behaviour should be: Check movements in the scene and if is something detected then save 30seconds of H264 video.

Default resolution is 1920x1080 for H264 video, for image processing only 320x240.
	
When is sensitivity too high/low - try to modify raspibgs.cfg file, interesting parameters can be
	
	[BGS]
	ObjectSizeX=3
	ObjectSizeY=3
	Threshold=9
		
Note: file raspibgs.cfg must be in directory where is raspibgs executed!

For built without OpenCV - modify host_applications/linux/apps/raspicam/CMakeLists.txt and remove following line: add_definitions(-D_D_USE_OPENCV)
In this case doesn't work motion detection!

Most time consuming part is taken data from GPU and prepare them for OpenCV which locate object in the scene, this can be defintely also made by GPU but now it will need more research.



