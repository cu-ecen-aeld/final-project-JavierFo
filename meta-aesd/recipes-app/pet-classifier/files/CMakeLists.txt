cmake_minimum_required(VERSION 3.10)
project(pet-classifier)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-app-1.0)

# Added mqtt_cnn.cpp
add_executable(pet-classifier main.cpp mqtt_cnn.cpp)

target_include_directories(pet-classifier PRIVATE 
    ${GST_INCLUDE_DIRS}
    ${STAGING_INCDIR}
)

# Added mosquitto
target_link_libraries(pet-classifier PRIVATE 
    ${GST_LIBRARIES} 
    tensorflow-lite 
    mosquitto
    pthread 
    dl)
