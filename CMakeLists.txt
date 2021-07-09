 cmake_minimum_required(VERSION 3.16.3)
 project(image_manipulation)

 # Tell cmake where the opencv build directory is installed on the computer
 # set(OpenCV_DIR <Path to your opencv library's build directory>) 

 # Tell cmake to store the final program executable in a bin directory
 set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")

 # Tell cmake to find the opencv library (with help of OPENCV_DIR)
 find_package(OpenCV REQUIRED)


 # Tell cmake that the executable will be called image_manipulation
 # and that it needs to compile ImageManipulation.cpp into a object file
 add_executable(image_manipulation ImageManipulation.cpp)

 # Link the openv lib directory to the object file
 target_link_libraries(image_manipulation ${OpenCV_LIBS}) 

 # Now the program should be compiled and linked, andt the executable will
 # be in the bin folder.
                                                              
