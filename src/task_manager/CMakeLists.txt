cmake_minimum_required(VERSION 3.15)

project(task_manager_lib LANGUAGES CXX)

add_library(task_manager INTERFACE)
target_include_directories(task_manager INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
