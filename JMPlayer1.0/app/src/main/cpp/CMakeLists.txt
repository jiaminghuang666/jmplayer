
# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.22.1)

# Declares and names the project.
#添加头文件路径（相对于本文件路径）
include_directories(include)

#设置ffmpeg库所在路径的变量
set(FF ${CMAKE_CURRENT_SOURCE_DIR}/libs/${ANDROID_ABI})
add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION ${FF}/libavcodec.so)

add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION ${FF}/libavfilter.so)

add_library(avformat SHARED IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION ${FF}/libavformat.so)

add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION ${FF}/libavutil.so)

add_library(swresample SHARED IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION ${FF}/libswresample.so)

add_library(swscale SHARED IMPORTED)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION ${FF}/libswscale.so)



project("JMPlayer")

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             JMPlayer

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             XParameter.cpp
             XData.cpp
             JMObserver.cpp
             XThread.cpp
             JMAudioPlay.cpp
             SLAudioPlay.cpp
             XEGL.cpp
             JMVideoView.cpp
             GLVideoView.cpp
             XTexture.cpp
             XShader.cpp
             TestOpenglEgl.cpp
             JMResample.cpp
             FFResample.cpp
             FFDecode.cpp
             JMDecode.cpp
             FFDemux.cpp
             JMDemux.cpp
             JMLog.cpp
             JMPlayerPorxy.cpp
             JMPlayer.cpp
             native-lib.cpp


        )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       JMPlayer
                       GLESv2 EGL
                       OpenSLES
                       android
                       avcodec avfilter avformat avutil swresample swscale

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )
