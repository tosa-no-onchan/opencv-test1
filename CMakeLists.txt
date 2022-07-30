cmake_minimum_required(VERSION 3.1)
project( opencv-test1 )
find_package( OpenCV REQUIRED )

add_executable( main-8 main-8.cpp )


target_link_libraries( main-8 ${OpenCV_LIBS} )

