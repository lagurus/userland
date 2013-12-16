
# raspistill/raspivid/raspiyuv

SET(COMPILE_DEFINITIONS -Werror)

add_definitions(-D_D_USE_OPENCV)


include_directories(${CMAKE_SOURCE_DIR}/host_applications/linux/libs/bcm_host/include)
include_directories(${CMAKE_SOURCE_DIR}/host_applications/linux/apps/raspicam/)

set (GL_SCENE_SOURCES
		gl_scenes/models.c
		gl_scenes/mirror.c
		gl_scenes/yuv.c
		gl_scenes/sobel.c
		gl_scenes/square.c
		gl_scenes/teapot.c
		gl_scenes/bgs_simple.cpp
   )

set (COMMON_SOURCES
   RaspiCamControl.c
   RaspiCLI.c
   RaspiPreview.c)

#add_executable(raspistill ${COMMON_SOURCES} RaspiStill.c  RaspiTex.c RaspiTexUtil.c tga.c Tools.c MemoryIniFile.c graphics.cpp ${GL_SCENE_SOURCES})
#add_executable(raspiyuv   ${COMMON_SOURCES} RaspiStillYUV.c)
#add_executable(raspivid   ${COMMON_SOURCES} RaspiVid.c RaspiTex.c RaspiTexUtil.c tga.c ${GL_SCENE_SOURCES})
add_executable(raspibgs   ${COMMON_SOURCES} RaspiBGS.c RaspiTex.c RaspiTexUtil.c tga.c Tools.c MemoryIniFile.c graphics.cpp ${GL_SCENE_SOURCES})

find_package( OpenCV REQUIRED )

set (MMAL_LIBS mmal_core mmal_util mmal_vc_client)

#target_link_libraries(raspistill ${MMAL_LIBS} vcos bcm_host GLESv2 EGL m)
#target_link_libraries(raspiyuv   ${MMAL_LIBS} vcos bcm_host)
#target_link_libraries(raspivid   ${MMAL_LIBS} vcos bcm_host GLESv2 EGL m)
target_link_libraries(raspibgs   ${MMAL_LIBS} vcos bcm_host GLESv2 EGL m ${OpenCV_LIBS})

#install(TARGETS raspistill raspiyuv raspivid RUNTIME DESTINATION bin)
install(TARGETS raspibgs RUNTIME DESTINATION bin)
