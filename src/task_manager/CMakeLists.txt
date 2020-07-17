cmake_minimum_required(VERSION 3.15)

project(task_manager LANGUAGES CXX)

add_library(task_manager STATIC main.cpp)
target_include_directories(task_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
