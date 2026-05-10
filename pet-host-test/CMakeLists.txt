cmake_minimum_required(VERSION 3.16)
project(pet-classifier-host)

set(CMAKE_CXX_STANDARD 17)

# Find GStreamer
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-app-1.0)

set(TFLITE_ENABLE_XNNPACK OFF CACHE BOOL "" FORCE)  # optional: speeds up build

# Fetch TensorFlow Lite from source
include(FetchContent)
FetchContent_Declare(
  tensorflow
  GIT_REPOSITORY https://github.com/tensorflow/tensorflow.git
  GIT_TAG        v2.11.0
  GIT_SHALLOW    TRUE
  SOURCE_SUBDIR  tensorflow/lite
)
FetchContent_MakeAvailable(tensorflow)

add_executable(pet-classifier-host host_main.cpp mqtt_cnn.cpp)

target_include_directories(pet-classifier-host PRIVATE 
    ${GST_INCLUDE_DIRS}
    ${tensorflow_SOURCE_DIR}
)

# Added mosquitto to linked libraries
target_link_libraries(pet-classifier-host PRIVATE 
    ${GST_LIBRARIES} 
    tensorflow-lite
    mosquitto
    pthread 
    dl
)
