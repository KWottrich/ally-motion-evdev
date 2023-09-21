cmake_minimum_required(VERSION 3.5)

project(iio-uinput)

set(SRC_FILES main.cpp uinputdev.cpp iioaccell.cpp iiogyro.cpp)
set(LIBS )

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries( ${PROJECT_NAME} ${LIBS} -lstdc++fs)
target_include_directories(${PROJECT_NAME} PRIVATE  "/usr/include/glib-2.0/" "/usr/lib/glib-2.0/include")
add_definitions(" -std=c++17 -Wall -Os -flto -fno-strict-aliasing")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")


set(CMAKE_INSTALL_PREFIX "/usr")
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
