

ADD_EXECUTABLE (${CMAKE_PROJECT_NAME} main.cpp STiFrameGrabber.cpp IFrame.cpp YUVFrame.cpp)
TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME} avformat avcodec swscale avutil)
