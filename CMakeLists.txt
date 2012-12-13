cmake_minimum_required(VERSION 2.8)
project(opencv_sandbox)

set(OpenCV_DIR "/home/evgeny/argus/git_trunk/build")

find_package(OpenCV REQUIRED)

add_executable(scenetext ./scenetext.cpp)
set_target_properties(scenetext PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(scenetext ${OpenCV_LIBS})
