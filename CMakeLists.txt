cmake_minimum_required(VERSION 3.1)
project( opencv-test1 )
find_package( OpenCV REQUIRED )
add_executable( main-8 main-8.cpp )
add_executable( main-9 main-9.cpp )
add_executable( main-10 main-10.cpp )


target_link_libraries( main-8 ${OpenCV_LIBS} )
target_link_libraries( main-9 ${OpenCV_LIBS} )
target_link_libraries( main-10 ${OpenCV_LIBS} )

