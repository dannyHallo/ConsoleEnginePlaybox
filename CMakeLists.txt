cmake_minimum_required(VERSION 3.23)

project("${PROJ_NAME}")

set(CMAKE_CXX_STANDARD 11)

if(NOT DEFINED PROJ_NAME)
  set(PROJ_NAME "DefaultProjectName")
endif()

add_executable(${PROJ_NAME} src/main.cc)